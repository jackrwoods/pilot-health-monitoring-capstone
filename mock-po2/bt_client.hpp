#pragma once

#include <iostream>
#include <string>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/l2cap.h>

namespace Bluetooth_Connection
{
    /*
    * Client: Bluetooth client
    * Code adapted (stolen) from https://people.csail.mit.edu/albert/bluez-intro/x559.html
    * Uses L2CAP sockets.
    */
    class Client
    {
    private:
    public:
        Client();
        Client(std::string addr);
        ~Client();
    };
} // namespace Bluetooth_Connection