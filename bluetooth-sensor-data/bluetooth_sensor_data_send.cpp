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
    std::cout << "Mock PO2 Sensor Send test" << std::endl;

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
        // return 1;
    }

    // spawn bluetooth communication in a new thread
    std::thread bt_thread(&PHMS_Bluetooth::Client::run, &c);

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
    // end bluetooth connection and thread
    c.quit();
    bt_thread.join();

    return 0;
}