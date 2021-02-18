#pragma once

#include <iostream>
#include <string>
#include <queue>
#include <mutex>
#include <unistd.h>

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

        bool connection_created{false};

        struct sockaddr_l2 socket_addr = {0};
        int s{0};
        int status{0};

        // end bluetooth variables

    public:
        Client();
        Client(std::string addr);
        ~Client();

        int open_con(std::string addr);
        int close_con();

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
    open_con(addr);
}

/**
 * Default destructor
 */
Bluetooth_Connection::Client::~Client()
{
    close_con();
}

/**
 * open: Creates a bluetooth connection.
 * @param addr The bluetooth address of device to connect to.
 * @returns 0 on success.
 */
int Bluetooth_Connection::Client::open_con(std::string addr)
{
    // allocate a socket
    s = socket(AF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);

    // set the connection parameters (who to connect to)
    socket_addr.l2_family = AF_BLUETOOTH;
    socket_addr.l2_psm = htobs(0x1001);
    str2ba(addr.c_str(), &socket_addr.l2_bdaddr);

    // connect to server
    status = connect(s, (struct sockaddr *)&socket_addr, sizeof(sockaddr_l2));

    if (status == 0)
        connection_created = true;

    return status;
}

/**
 * close: Closes existing bluetooth connection.
 * @return 0 on success.
 */
int Bluetooth_Connection::Client::close_con()
{
    close(s);
    connection_created = false;
    return 0;
}

/**
 * push: Add a packet to the queue to be transmitted over bluetooth.
 * @param p The packet to be transmitted.
 * @returns True if a connection was created, false otherwise.
 */
void Bluetooth_Connection::Client::push(const Bluetooth_Connection::Packet &p)
{
    pkt_guard.lock();
    pkt_queue.push(p);
    pkt_guard.unlock();
}

/**
 * quit: Stops execution of the run() function.
 */
void Bluetooth_Connection::Client::quit()
{
    is_quit = true;
}

/**
 * run: Thread entry point. Sends packets from the pkt_queue until quit() is called.
 */
void Bluetooth_Connection::Client::run()
{
    // do not attempt to transmit data if a connection has not been created.
    if (connection_created == false)
        return;

    while (is_quit == false)
    {
        if (pkt_queue.size() > 0)
        {
            // get the front packet from the queue
            Packet p(pkt_queue.front());
            pkt_queue.pop();

            // attempt to send the packet's data over bluetooth
            int status = write(s, p.get(), p.size());
            if (status < 0)
            {
                std::cerr << "An error occurred transmitting packet (CLIENT)" << std::endl;
            }
        }
    }
}