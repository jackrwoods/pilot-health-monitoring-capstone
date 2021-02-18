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

/**
 * Default constructor.
 */
Bluetooth_Connection::Client::Client()
{
}

/**
 * Constructor with connection creation.
 * @param addr Bluetooth address to connect to.
 */
Bluetooth_Connection::Client::Client(std::string addr)
{
}

/**
 * Default destructor
 */
Bluetooth_Connection::Client::~Client()
{
}

/**
 * open: Creates a bluetooth connection.
 * @param addr The bluetooth address of device to connect to.
 * @returns 0 on success.
 */
int Bluetooth_Connection::Client::open(std::string addr)
{
}

/**
 * close: Closes existing bluetooth connection.
 * @return 0 on success.
 */
int Bluetooth_Connection::Client::close()
{
}

/**
 * push: Add a packet to the queue to be transmitted over bluetooth.
 * @param p The packet to be transmitted.
 * @returns True if a connection was created, false otherwise.
 */
void Bluetooth_Connection::Client::push(const Bluetooth_Connection::Packet &p)
{
}

/**
 * quit: Stops execution of the run() function.
 */
void Bluetooth_Connection::Client::quit()
{
}

/**
 * run: Thread entry point. Sends packets from the pkt_queue until quit() is called.
 */
void Bluetooth_Connection::Client::run()
{
}