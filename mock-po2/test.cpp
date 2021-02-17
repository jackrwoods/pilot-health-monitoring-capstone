#include <iostream>

#include "./bluetooth_con.hpp"

int main(int argc, char *argv[])
{
    std::cout << "test" << std::endl;

    auto cons = Bluetooth_Connection::available_connections();
    for (auto i = cons.begin(); i != cons.end(); i++)
    {
        printf("- %s (MAC: %s)\n", i->name, i->address);
    }
}