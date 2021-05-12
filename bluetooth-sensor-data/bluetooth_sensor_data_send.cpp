#include <iostream>
#include <thread>
#include <fstream>
#include "./scoped_screen.hpp"

#include "../data-server/include/bluetooth/bluetooth_con.hpp"
#include "../data-server/include/bluetooth_utils.hpp"
#include "../data-server/include/datasource.hpp"

// samples per second to send
#define SAMPLE_RATE 64

// samples per packet
#define PACKET_SIZE 5

int main(int argc, char *argv[])
{
    // std::cout << "ECE Black Box Simulator\n";

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

    // start ncurses mode
    Scoped_Screen s;


    // spawn bluetooth communication in a new thread
    c.run();

    // read data from input file
    std::vector<Sample> samples = sample_buffer_from_file(argv[2]);

    int total_samples{0};
    int packets_sent{0};
    int pilot_states_received{0};
    int last_pilot_state{0};
    const char *pilot_states[4] = {
        "unstressed",
        "stressed",
        "extra stressed",
        "unknown"
    };
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
        packets_sent++;
        total_samples += this_round.size();

        if(c.available())
        {
            auto v = c.get_all();
            pilot_states_received += v.size();
            for(auto vi : v)
                switch(vi.get()[0])
                {
                    case(0):
                        // std::cout << "Received pilot state unstressed\n";
                        last_pilot_state = 0;
                        break;
                    case(1):
                        // std::cout << "Received pilot state stressed\n";
                        last_pilot_state = 1;
                        break;
                    case(2):
                        // std::cout << "Received pilot state stressed (extra bad)\n";
                        last_pilot_state = 2;
                        break;
                    default:
                        // std::cout << "Received unknown pilot state\n";
                        last_pilot_state = 3;
                }
        }
        clear();
        mvprintw(0, 0, "ECE Black Box");
        mvprintw(1, 0, "Connected to device at (server): %s (client): %s", c.get_server_connected_address().c_str(), c.get_client_connected_address().c_str());
        mvprintw(2, 0, "Packets sent: %i", packets_sent);
        mvprintw(3, 0, "Samples sent: %i", total_samples);
        mvprintw(4, 0, "Pilot states recieved: %i" , pilot_states_received);
        mvprintw(5, 0, "Last received pilot state: %s", pilot_states[last_pilot_state]);
        
        refresh();
    }

    // std::cout << "Sent " << total_samples << " samples.\n";
    // std::cout << "Sample send finished. Receive thread may still be receiving samples. Enter key to quit...\n";
    mvprintw(7, 0, "Sample sending finished. Receive thread may still be receiving samples. Press any key to quit...\n");
    getch();

    // std::cin.getline(nullptr, 0);

    // end bluetooth connection and thread
    c.quit();

    std::cout << "Received " << c.available() << " packets.\n";

    return 0;
}
