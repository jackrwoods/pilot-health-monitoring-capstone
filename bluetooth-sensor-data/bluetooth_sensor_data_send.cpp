#include <iostream>
#include <thread>
#include <fstream>

#include "./bluetooth/bluetooth_con.hpp"
#include "./sample_types.hpp"

// samples per second to send
#define SAMPLE_RATE 64

// samples per packet
#define PACKET_SIZE 5

bool transmission_quit {false};

void send_packets(PHMS_Bluetooth::Client *cl, std::string filename)
{
    PHMS_Bluetooth::Client &c = *cl;
    // read data from input file
    std::vector<Sample> samples = sample_buffer_from_file(filename);

    int total_samples{0};

    auto start_time = std::chrono::high_resolution_clock::now();
    while (!transmission_quit)
    {
        for (int i = 0; i + PACKET_SIZE < samples.size(); i += PACKET_SIZE)
        {
            if(transmission_quit)
                break;

            // measure the execution time of pushing to bluetooth client
            auto inner_start_time = std::chrono::high_resolution_clock::now();

            // construct vector of this round's samples to be transmitted
            std::vector<Sample> this_round;
            this_round.insert(this_round.begin(), samples.begin() + i, samples.begin() + i + PACKET_SIZE);
            
            c.push(packet_from_Sample_buffer(this_round));


            auto inner_end_time = std::chrono::high_resolution_clock::now();
            
            auto inner_duration = std::chrono::duration_cast<std::chrono::microseconds>(inner_end_time - inner_start_time);
            std::cout << "Pushed " << this_round.size() << " samples to bluetooth in " << inner_duration.count() << " microseconds...\n";

            // calculate approximate sleep for desired sample rate with packet size to reach SAMPLE_RATE
            // (1000000 / SAMPLE_RATE) => microseconds spent per sample
            // * PACKET_SIZE, microseconds spent per packet
            // subtract recorded time
            usleep((1000000 / SAMPLE_RATE) * PACKET_SIZE - inner_duration.count());
            total_samples += this_round.size();
        }
    }
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);

    std::cout << "Sent " << total_samples << " samples at " << total_samples / (duration.count() / 1000000.0f) << " hz\n";
    // end bluetooth connection and thread
}

int main(int argc, char *argv[])
{
    std::cout << "Mock Sensor Data Send" << std::endl;
    std::cout << "Press Enter when done" << std::endl;

    if (argc != 3)
    {
        std::cerr << "Exactly two command line arguments required: ./bluetooth_sensor_data_send.out [blueooth address] [input file]" << std::endl;
        return 1;
    }

    // open bluetooth connection
    PHMS_Bluetooth::Client c;
    int con_stat = c.open_con(std::string(argv[1]));
    if (con_stat == -1)
    {
        std::cerr << "Error opening connection to bluetooth device at " << argv[1] << '.' << std::endl;
        return 1;
    }

    // spawn bluetooth communication in a new thread
    std::thread bt_thread(&PHMS_Bluetooth::Client::run, &c);

    std::thread send_thread(send_packets, &c, argv[2]);

    std::cin.getline(nullptr, 0);
    transmission_quit = true;

    send_thread.join();
    c.quit();
    bt_thread.join();

    return 0;
}