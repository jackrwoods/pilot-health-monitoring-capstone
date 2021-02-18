#include <iostream>
#include <thread>

#include "./bt_server.hpp"

int main()
{
    std::cout << "Server Test" << std::endl;
    Bluetooth_Connection::Server s;
    s.open_con("");

    std::thread server(&Bluetooth_Connection::Server::run, &s);

    std::string q;
    std::getline(std::cin, q);

    std::vector<Bluetooth_Connection::Packet> packets = s.get_all();

    for(auto i = packets.begin(); i != packets.end(); i++)
        i->print();

    s.quit();
    server.join();

    return 0;
}