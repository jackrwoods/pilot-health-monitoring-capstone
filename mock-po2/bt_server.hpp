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

/**
 * Default constructor.
 */
Bluetooth_Connection::Server::Server()
{
}

/**
 * Constructor with connection creation.
 * @param addr Bluetooth address to connect to.
 */
Bluetooth_Connection::Server::Server(std::string addr)
{
}

/**
 * Default destructor
 */
Bluetooth_Connection::Server::~Server()
{
}

/**
 * open: Creates a bluetooth connection.
 * @param addr The bluetooth address of device to connect to.
 * @returns 0 on success.
 */
int Bluetooth_Connection::Server::open(std::__cxx11::string addr)
{
}

/**
 * close: Closes existing bluetooth connection.
 * @return 0 on success.
 */
int Bluetooth_Connection::Server::close()
{
}

/**
 * available: Get the number of available Packets.
 * @return Number of available Packets.
 */
size_t Bluetooth_Connection::Server::available()
{
}

/**
 * quit: Stops execution of the run() function.
 */
void Bluetooth_Connection::Server::quit()
{
}

/**
 * run: Thread entry point. Receives packets until quit() is called.
 */
void Bluetooth_Connection::Server::run()
{
}