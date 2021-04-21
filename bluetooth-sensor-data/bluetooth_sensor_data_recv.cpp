#include <iostream>
#include <thread>

#include "./bluetooth/bluetooth_con.hpp"
#include "./sample_types.hpp"

int main(int argc, char *argv[])
{
    std::cout << "Mock PO2 Sensor Recieve test ('hcitool dev' in terminal returns the bluetooth address of this device)" << std::endl;



    PHMS_Bluetooth::Server s;
    s.open_con();

    std::thread bt_thread(&PHMS_Bluetooth::Server::run, &s);

    // receive from bluetooth

    

    s.quit();
    bt_thread.join();

    return 0;
}