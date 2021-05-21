#include <iostream>
#include <thread>

#include "./bt_server.hpp"

int main()
{
    std::cout << "Server Test" << std::endl;
    PHMS_Bluetooth::Server s;
    s.open_con();

    std::thread server(&PHMS_Bluetooth::Server::run, &s);

    std::string q;
    std::getline(std::cin, q);

    std::vector<PHMS_Bluetooth::Packet> packets = s.get_all();

    for(auto i = packets.begin(); i != packets.end(); i++)
        i->print();

    s.quit();
    server.join();

    return 0;
}