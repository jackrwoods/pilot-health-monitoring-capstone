#include <iostream>
#include <thread>
#include <unistd.h>
#include <chrono>

#include "./bluetooth/bluetooth_con.hpp"

// samples per second to send
#define SAMPLE_RATE 64

// samples per packet
#define PACKET_SIZE 32

// how many seconds should data be sent for
#define SECONDS 30

// identify what kind of data is in a packet
#define ID_SAMPLE 0xFF

#define ID_ECE_BPM 0xFE
#define ID_ECE_PO2 0xFD

int main(int argc, char *argv[])
{
    std::cout << "Data IO block tests - Send Samples at 64 hz and receive Pilot States at 64 hz\n";

    if (argc != 2)
    {
        std::cerr << "Exactly one command line argument required: ./test_client.out [blueooth address]" << std::endl;
        return 1;
    }

    // open bluetooth connections

    // client
    PHMS_Bluetooth::Client c;
    int con_stat = c.open_con(std::string(argv[1]));
    if (con_stat == -1)
    {
        std::cerr << "Error opening connection to bluetooth device at " << argv[1] << '.' << std::endl;
        return 1;
    }
    // spawn bluetooth client in a new thread
    std::thread client(&PHMS_Bluetooth::Client::run, &c);

    // server in a new thread
    PHMS_Bluetooth::Server s;
    s.open_con();
    std::thread server(&PHMS_Bluetooth::Server::run, &s);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Send Samples and ECE measurements.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // write data here for all tests
    uint32_t buffer[PACKET_SIZE * 2 + 1]{0};

    // SAMPLE PACKETS =================================================================================================
    // arbitrary. Defines sample packet
    buffer[0] = ID_SAMPLE;

    std::cout << "Transmitting " << (SAMPLE_RATE * SECONDS) << " samples generated at " << SAMPLE_RATE << " hertz to " << argv[1] << "..." << std::endl;

    for (int round = 0; round < (SAMPLE_RATE * SECONDS) / PACKET_SIZE; round++)
    {
        // time data generation - should be 64hz
        auto start = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < PACKET_SIZE * 2; i += 2)
        {
            // bpm sample
            buffer[i + 1] = i;
            // po2 sample
            buffer[i + 2] = i;
            usleep(995000 / SAMPLE_RATE);
        }
        c.push(buffer, PACKET_SIZE * 4);

        auto end = std::chrono::high_resolution_clock::now();
        auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() / PACKET_SIZE;
        float hz = 1000000000.0 / nanos;

        std::cout << "sent packet with data generation of " << hz << " hertz." << std::endl;
    }

    // BPM MEASUREMENTS ===============================================================================================
    buffer[0] = ID_ECE_BPM; // arbitrary. Defines bpm measurement packet

    // transmit bpm of 7
    int bpm{7};
    buffer[1] = bpm;

    c.push(buffer, 8);

    std::cout << "sent packet with bpm measurement (" << bpm << ")\n";

    // PO2 MEASUREMENTS ===============================================================================================
    buffer[0] = ID_ECE_PO2; // arbitrary. Defines po2 measurement packet

    // transmit po2 of 7
    int po2{7};
    buffer[1] = po2;

    c.push(buffer, 8);

    std::cout << "sent packet with po2 measurement (" << po2 << ")\n";
    std::cout << "Finished sending." << std::endl;

    std::cout << "Beginning begin Pilot State tests.\n";

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Receive Pilot States.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // Pilot State tests

    // Once everything has been received
    std::cout << "Press enter once all samples have been transmitted. (1)\n";
    std::cin.getline(nullptr, 0);

    auto v = s.get_all();

    int state_count{0};

    for (auto i = v.begin(); i != v.end(); i++)
        state_count++;

    std::cout << "Received " << state_count << " Pilot States.\n";

    // end bluetooth connections and threads
    c.close_con();
    c.quit();
    client.join();

    s.close_con();
    s.quit();
    server.join();

    return 0;
}
