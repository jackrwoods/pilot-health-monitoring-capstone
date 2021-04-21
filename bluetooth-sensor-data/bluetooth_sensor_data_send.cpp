#include <iostream>
#include <thread>
#include <fstream>

#include "./bluetooth/bluetooth_con.hpp"
#include "./sample_types.hpp"

// samples per second to send
#define SAMPLE_RATE 64

// samples per packet
#define PACKET_SIZE 32


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
        return 1;
    }

    // open file for reading
    std::ifstream input_file(argv[2], std::ios::in);
    if(!input_file.is_open())
    {
        std::cerr << "Error opening file " << argv[2] << "." << std::endl;
        return 1;
    }


    // spawn bluetooth communication in a new thread
    std::thread bt_thread(&PHMS_Bluetooth::Client::run, &c);


    // read data from input file, send



    // end bluetooth connection and thread
    c.quit();
    bt_thread.join();

    return 0;
}