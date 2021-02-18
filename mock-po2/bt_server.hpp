#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <mutex>

#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/l2cap.h>

#include "./bt_packet.hpp"

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
        std::vector<Packet> pkt_buffer;
        std::mutex pkt_guard;

        bool is_quit{false};

        // bluetooth variables



        // end bluetooth variables

    public:
        Server();
        Server(std::string addr);
        ~Server();

        int open(std::string addr);
        int close();

        size_t available();
        std::vector<Packet> get_all();

        void quit();

        void run();
    };
} // namespace Bluetooth_Connection