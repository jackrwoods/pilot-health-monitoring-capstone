#pragma once

#include <iostream>
#include <string>
#include <queue>
#include <mutex>

#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/l2cap.h>

#include "./bt_packet.hpp"

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
        std::queue<Packet> pkt_queue;
        std::mutex pkt_guard;

        bool is_quit{false};

        // bluetooth variables



        // end bluetooth variables

    public:
        Client();
        Client(std::string addr);
        ~Client();

        int open(std::string addr);
        int close();

        void push(const Packet &p);

        void quit();

        void run();
    };
} // namespace Bluetooth_Connection