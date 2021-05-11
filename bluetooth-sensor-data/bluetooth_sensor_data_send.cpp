#include <iostream>
#include <thread>
#include <fstream>

#include "../data-server/include/bluetooth/bluetooth_con.hpp"
#include "../data-server/include/bluetooth_utils.hpp"
#include "../data-server/include/datasource.hpp"

// samples per second to send
#define SAMPLE_RATE 64

// samples per packet
#define PACKET_SIZE 5

int main(int argc, char *argv[])
{
    std::cout << "ECE Black Box Simulator\n";

    if (argc != 3)
    {
        std::cerr << "Exactly two command line arguments required: ./bluetooth_sensor_data_send.out [blueooth address] [input file]" << std::endl;
        return 1;
    }

    // open bluetooth connection
    PHMS_Bluetooth::Communicator c;
    int con_stat = c.open_con(std::string(argv[1]), 5);
    if (con_stat != 0)
    {
        std::cerr << "Error opening connection to bluetooth device at " << argv[1] << '.' << std::endl;
        return 1;
    }

    // spawn bluetooth communication in a new thread
    c.run();

    // read data from input file
    std::vector<Sample> samples = sample_buffer_from_file(argv[2]);

    int total_samples{0};

    while (samples.size() > 0)
    {
        std::vector<Sample> this_round;
        if (samples.size() > PACKET_SIZE)
        {
            this_round.insert(this_round.begin(), samples.begin(), samples.begin() + PACKET_SIZE);
            samples.erase(samples.begin(), samples.begin() + PACKET_SIZE);
        }
        else
        {
            this_round.insert(this_round.begin(), samples.begin(), samples.end());
            samples.clear();
        }
        // calculate approximate sleep for desired sample rate with packet size
        usleep(1000000 / (SAMPLE_RATE / PACKET_SIZE));
        c.push(packet_from_Sample_buffer(this_round));
        std::cout << "Pushing " << this_round.size() << " samples\n";
        total_samples += this_round.size();
    }

    std::cout << "Sent " << total_samples << " samples.\n";
    std::cout << "Sample send finished. Receive thread may still be receiving samples. Enter key to quit...\n";

    std::cin.getline(nullptr, 0);

    // end bluetooth connection and thread
    c.quit();

    std::cout << "Received " << c.available() << " packets.\n";

    return 0;
}