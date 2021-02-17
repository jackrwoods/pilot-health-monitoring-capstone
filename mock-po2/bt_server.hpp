#pragma once

#include <iostream>
#include <string>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/l2cap.h>

namespace Bluetooth_Connection
{
    /*
    * Server: Bluetooth server
    * Code adapted (stolen) from https://people.csail.mit.edu/albert/bluez-intro/x559.html
    * Uses L2CAP sockets.
    */
    class Server
    {
    private:
    public:
        Server();
        Server(std::string addr);
        ~Server();
    };
} // namespace Bluetooth_Connection