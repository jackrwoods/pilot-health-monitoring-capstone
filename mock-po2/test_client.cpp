#include <iostream>
#include <thread>

#include "./bt_client.hpp"

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "Exactly two arguments required: ./test_client.out [blueooth address]" << std::endl;
        return 1;
    }

    std::cout << "Client Test" << std::endl;
    Bluetooth_Connection::Client c;
    
    std::cout << "Opening connection: " << c.open_con(std::string(argv[1])) << std::endl;

    std::thread client(&Bluetooth_Connection::Client::run, &c);

    std::string in;
    while (in != "q")
    {
        std::getline(std::cin, in);
        Bluetooth_Connection::Packet p(in.length(), in.c_str());
        c.push(p);
    }

    c.quit();
    client.join();

    return 0;
}