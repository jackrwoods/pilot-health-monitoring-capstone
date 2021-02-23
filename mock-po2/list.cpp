#include <iostream>

#include "./bluetooth/bluetooth_con.hpp"

int main(int argc, char *argv[])
{
    std::cout << "List available Bluetooth devices" << std::endl;

    auto cons = PHMS_Bluetooth::available_connections();
    for (auto i = cons.begin(); i != cons.end(); i++)
    {
        printf("- %s (MAC: %s)\n", i->name, i->address);
    }
}