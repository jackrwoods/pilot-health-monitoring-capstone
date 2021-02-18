#include <iostream>
#include <thread>

#include "./bt_client.hpp"

int main()
{
    std::cout << "Client Test" << std::endl;
    Bluetooth_Connection::Client c;
    c.open_con("");

    std::thread client(&Bluetooth_Connection::Client::run, &c);

    std::string in;
    while(in != "q") {
        std::getline(std::cin, in);
        Bluetooth_Connection::Packet p(in.length(), in.c_str());
        c.push(p);
    }

    c.quit();
    client.join();

    return 0;
}