#include <iostream>
#include <thread>
#include <fstream>
#include <deque>
#include <mutex>

#include "../data-server/include/bluetooth/bluetooth_con.hpp"
#include "../data-server/include/bluetooth_utils.hpp"
#include "../data-server/include/datasource.hpp"

#include "./scoped_screen.hpp"
#include "./mock_sensor.hpp"

// samples per second to send
#define SAMPLE_RATE 64

// samples per packet
#define PACKET_SIZE 5

// struct for holding each sensor and its associated data
struct Sensor_Data
{
    Mock_Sensor sensor;
    std::deque<Sample> samples;
    bool set_valid{true};

    Sensor_Data(Inner_Sensor *i);
};
Sensor_Data::Sensor_Data(Inner_Sensor *i) : sensor(i){};

// GLOBAL VARIABLES - EASY ACCESS TO ALL THREADS
int total_samples{0};
int total_valid_samples{0};
int packets_sent{0};

int pilot_states_received{0};
int last_pilot_state{0};
int current_pilot_stress{0};
const char *pilot_states[4] = {
    "unstressed",
    "stressed",
    "extra stressed",
    "unknown"};

PHMS_Bluetooth::Communicator c;

std::mutex sensor_guard;
std::vector<Sensor_Data> sensors;

bool global_is_quit{false};

//ui thread - allow user to adjust settings and view data stats
void ui(Inner_Sensor *i)
{
    Inner_Sensor &inner_sensor = *i;

    // start ncurses mode
    Scoped_Screen s;

    int selected_sensor{0};

    while (!global_is_quit)
    {
        // easier way to print controls
        std::array<const char *, 10> controls = {{"q: quit",
                                                  "+: increase stress level",
                                                  "-: decrease stress level",
                                                  "arrow keys: select a sensor",
                                                  "a: add a sensor",
                                                  "d: delete a sensor",
                                                  "v: toggle sensor validity",
                                                  ":",
                                                  ":",
                                                  ":"}};

        clear();

        // print general information
        mvprintw(0, 0, "ECE Black Box");
        mvprintw(1, 0, "Connected to device at (server): %s (client): %s", c.get_server_connected_address().c_str(), c.get_client_connected_address().c_str());
        mvprintw(2, 0, "Packets sent: %i", packets_sent);
        mvprintw(3, 0, "Samples sent: %i", total_samples);
        mvprintw(4, 0, "Pilot states recieved: %i", pilot_states_received);
        mvprintw(5, 0, "Last received pilot state: %s", pilot_states[last_pilot_state]);
        mvprintw(6, 0, "Current pilot stress level %i/10", current_pilot_stress);
        refresh();

        // print control instructions
        for (int i = 0; i < controls.size(); i++)
            mvprintw(10 + i, 0, "%s", controls[i]);

        // print sensor information
        for (int i = 0; i < sensors.size(); i++)
            mvprintw(25 + i, 0, "%s %i: %s", (i == selected_sensor) ? "*Sensor" : " Sensor", i, (sensors[i].set_valid) ? "valid" : "invalid");

        // get input - change state based on input
        int c = getch();
        switch (c)
        {
        case 'q':
            global_is_quit = true;
            break;

        case '+':
            if (current_pilot_stress < 10)
                ++current_pilot_stress;
            for (auto i : sensors)
                i.sensor.set_stress_level(current_pilot_stress);
            break;
        case '-':
            if (current_pilot_stress > 0)
                --current_pilot_stress;
            for (auto i : sensors)
                i.sensor.set_stress_level(current_pilot_stress);
            break;

        case KEY_UP:
            if (selected_sensor > 0)
                --selected_sensor;
            break;

        case KEY_DOWN:
            if (selected_sensor < sensors.size() - 1)
                ++selected_sensor;
            break;

        case 'a':
            // dont add more than 255 sensors
            if (sensors.size() == 0xff)
                break;
            sensor_guard.lock();
            sensors.push_back(Sensor_Data(&inner_sensor));
            sensors.back().sensor.set_stress_level(current_pilot_stress);
            sensor_guard.unlock();
            break;

        case 'd':
            if (sensors.size() > 1)
            {
                sensor_guard.lock();
                sensors.erase(sensors.begin() + selected_sensor);
                sensor_guard.unlock();
                selected_sensor = 0;
            }
            break;

        case 'v':
            if (sensors[selected_sensor].set_valid)
            {
                sensors[selected_sensor].sensor.make_invalid();
                sensors[selected_sensor].set_valid = false;
            }
            else
            {
                sensors[selected_sensor].sensor.make_valid();
                sensors[selected_sensor].set_valid = true;
            }

        default:
            break;
        }
    }
}

// data thread - control sensors
int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        std::cerr << "Exactly three command line arguments required: " << argv[0] << " [blueooth address] [stressed input file] [unstressed input file]" << std::endl;
        return 1;
    }

    std::string bluetooth_address = argv[1];
    std::string stressed_filename = argv[2];
    std::string unstressed_filename = argv[3];

    // this gets passed to each mock_sensor to ensure each sensor begins with the same values csv
    static Inner_Sensor inner_sensor(stressed_filename, unstressed_filename);

    // open bluetooth connection
    int con_stat = c.open_con(bluetooth_address, 5);
    if (con_stat != 0)
    {
        std::cerr << "Error opening connection to bluetooth device at " << bluetooth_address << '.' << std::endl;
        return 1;
    }

    // spawn bluetooth communication in a new thread
    c.run();

    sensors.push_back(Sensor_Data(&inner_sensor));

    // ui thread
    std::thread ui_thread(&ui, &inner_sensor);

    while (!global_is_quit)
    {

        // calculate approximate sleep for desired sample rate with packet size
        usleep(1000000 / (SAMPLE_RATE / PACKET_SIZE));

        // grab samples from each sensor
        // send all samples over bluetooth
        int valid_sensor_count{0};
        sensor_guard.lock();
        for (int i = 0; i < sensors.size(); i++)
        {
            // construct bluetooth packet from sensor data
            std::vector<Sample> samples = sensors[i].sensor.get(PACKET_SIZE);
            PHMS_Bluetooth::Packet pkt = packet_from_Sample_buffer(i, samples);

            c.push(pkt);

            // update variables
            packets_sent++;
            total_samples += PACKET_SIZE;
            if (sensors[i].set_valid)
                valid_sensor_count++;
        }
        total_valid_samples += (valid_sensor_count * PACKET_SIZE);
        sensor_guard.unlock();

        // update sensor position
        inner_sensor.inc_pos(PACKET_SIZE);

        // set pilot state variables
        if (c.available())
        {
            auto v = c.get_all();
            pilot_states_received += v.size();
            for (auto vi : v)
                switch (vi.get()[0])
                {
                case (0):
                    last_pilot_state = 0;
                    break;
                case (1):
                    last_pilot_state = 1;
                    break;
                case (2):
                    last_pilot_state = 2;
                    break;
                default:
                    last_pilot_state = 3;
                }
        }
    }

    // end bluetooth connection and thread
    c.quit();

    ui_thread.join();

    std::cout << "Packets sent: " << packets_sent << std::endl;
    std::cout << "Samples sent: " << total_samples << std::endl;
    std::cout << "Total valid samples sent: " << total_valid_samples << std::endl;
    std::cout << "Pilot states received: " << pilot_states_received << std::endl;

    return 0;
}
