#include <iostream>
#include <thread>

#include "./bluetooth/bluetooth_con.hpp"
#include "./sample_types.hpp"

bool quit_receive_thread{false};
int received_samples{0};

void run_receive()
{
    PHMS_Bluetooth::Server s;
    s.open_con();
    std::thread bt_thread(&PHMS_Bluetooth::Server::run, &s);

    while (!quit_receive_thread)
    {

        if (s.available())
        {
            // grab all available bluetooth packets
            std::vector<PHMS_Bluetooth::Packet> v = s.get_all();
            received_samples += v.size();

            // for each bluetooth packet received, get the samples, print them
            for (auto i : v)
            {
                std::vector<Sample> samples = sample_buffer_from_bt_packet(i);
                for (auto s : samples)
                    print(s);
            }
        }
    }
    s.quit();
    bt_thread.join();
}

int main(int argc, char *argv[])
{
    std::cout << "Mock PO2 Sensor Recieve test ('hcitool dev' in terminal returns the bluetooth address of this device)" << std::endl;

    // run a thread that sits and receives packets until user provides input
    std::thread main_thread(run_receive);

    std::cin.getline(nullptr, 0);

    quit_receive_thread = true;

    main_thread.join();

    std::cout << "received " << received_samples << " samples.\n";

    return 0;
}