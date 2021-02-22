#include <iostream>
#include <fstream>
#include <thread>

#include "./bluetooth/bluetooth_con.hpp"

int main(int argc, char *argv[])
{
    std::cout << "Mock PO2 Sensor Recieve test ('hcitool dev' in terminal returns the bluetooth address of this device)" << std::endl;

    // create received_po2_data.csv - all recevied data will be written to this file
    std::ofstream file("received_po2_data.csv", std::ios::trunc | std::ios::out);

    PHMS_Bluetooth::Server s;
    s.open_con();

    std::thread server(&PHMS_Bluetooth::Server::run, &s);

    std::string q;
    std::getline(std::cin, q);

    std::vector<PHMS_Bluetooth::Packet> packets = s.get_all();

    std::cout << "Received bluetooth packets (written to file received_po2_data.csv)" << std::endl;
    for (auto i = packets.begin(); i != packets.end(); i++)
    {
        // i->print();
        for (int j = 0; j < i->size() / 4; j++)
        {
            file << reinterpret_cast<const uint32_t *>(i->get())[j];
            file << "\n";
        }
    }

    s.quit();
    server.join();

    return 0;
}