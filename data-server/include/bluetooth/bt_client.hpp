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

namespace PHMS_Bluetooth
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

        std::string connected_address;

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

        std::string get_connected_address();

        void push(const Packet &p);
        void push(const void* src, size_t len);

        void quit();

        void run();

    };
} // namespace PHMS_Bluetooth

/**
 * Default constructor.
 */
PHMS_Bluetooth::Client::Client()
{
}

/**
 * Constructor with connection creation.
 * @param addr Bluetooth address to connect to.
 */
PHMS_Bluetooth::Client::Client(std::string addr)
{
    open_con(addr);
}

/**
 * Default destructor
 */
PHMS_Bluetooth::Client::~Client()
{
    close_con();
}

/**
 * open: Creates a bluetooth connection.
 * @param addr The bluetooth address of device to connect to.
 * @returns 0 on success.
 */
int PHMS_Bluetooth::Client::open_con(std::string addr)
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
    {
        connection_created = true;
        connected_address = addr;
        fprintf(stderr, "(Bluetooth Client) connected to %s\n", connected_address.c_str());
    }

    return status;
}

/**
 * close: Closes existing bluetooth connection.
 * @return 0 on success.
 */
int PHMS_Bluetooth::Client::close_con()
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
void PHMS_Bluetooth::Client::push(const PHMS_Bluetooth::Packet &p)
{
    pkt_guard.lock();
    pkt_queue.push(p);
    pkt_guard.unlock();
}

/**
 * push: Add data to be transmitted over bluetooth.
 * @param src Location of data to be put into packet.
 * @param len Number of bytes to be transmitted.
 */
void PHMS_Bluetooth::Client::push(const void *src, size_t len) {
    push(PHMS_Bluetooth::Packet(len, src));
}

/**
 * quit: Stops execution of the run() function.
 */
void PHMS_Bluetooth::Client::quit()
{
    is_quit = true;
}

/**
 * run: Thread entry point. Sends packets from the pkt_queue until quit() is called.
 */
void PHMS_Bluetooth::Client::run()
{
    // do not attempt to transmit data if a connection has not been created.
    if (connection_created == false)
        return;

    while (is_quit == false)
    {
        if (pkt_queue.size() > 0)
        {
            // get the front packet from the queue
            pkt_guard.lock();
            Packet p(pkt_queue.front());
            pkt_queue.pop();
            pkt_guard.unlock();

            // attempt to send the packet's data over bluetooth
            int status = write(s, p.get(), p.size());
            if (status < 0)
            {
                std::cerr << "An error occurred transmitting packet (CLIENT)" << std::endl;
            }
        }
    }
}

std::string PHMS_Bluetooth::Client::get_connected_address()
{
    return connected_address;
}