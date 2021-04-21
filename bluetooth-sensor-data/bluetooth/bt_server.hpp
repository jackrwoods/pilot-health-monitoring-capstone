#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <mutex>
#include <unistd.h>

#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/l2cap.h>

#include "./bt_packet.hpp"

#define MAX_PKT_SIZE 1024

namespace PHMS_Bluetooth
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

        bool connection_created{false};

        // bluetooth variables

        struct sockaddr_l2 loc_addr = {0};
        struct sockaddr_l2 rem_addr = {0};
        int s{0};
        int client{0};
        int bytes_read{0};
        socklen_t opt{sizeof(sockaddr_l2)};

        // end bluetooth variables

    public:
        ~Server();

        int open_con();
        int close_con();

        size_t available();
        std::vector<Packet> get_all();

        void quit();

        void run();
    };
} // namespace PHMS_Bluetooth

/**
 * Default destructor
 */
PHMS_Bluetooth::Server::~Server()
{
    close_con();
}

/**
 * open: Creates a bluetooth connection.
 * @param addr The bluetooth address of device to connect to.
 * @returns 0 on success.
 */
int PHMS_Bluetooth::Server::open_con()
{
    if(connection_created == true)
        return -1;

    // allocate socket
    s = socket(AF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);

    // bind socket to port 0x1001 of the first available
    // bluetooth adapter
    bdaddr_t tmp = {{0, 0, 0, 0, 0, 0}};
    loc_addr.l2_family = AF_BLUETOOTH;
    loc_addr.l2_bdaddr = tmp;
    loc_addr.l2_psm = htobs(0x1001);

    bind(s, (struct sockaddr *)&loc_addr, sizeof(sockaddr_l2));

    // put socket into listening mode
    listen(s, 1);

    // accept one connection
    client = accept(s, (struct sockaddr *)&rem_addr, &opt);
    if (client != -1)
    {
        connection_created = true;
        char buffer[256]{0};
        ba2str(&rem_addr.l2_bdaddr, reinterpret_cast<char *>(buffer));
        fprintf(stderr, "accepted connection from %s\n", buffer);
    }

    return client;
}

/**
 * close: Closes existing bluetooth connection.
 * @return 0 on success.
 */
int PHMS_Bluetooth::Server::close_con()
{
    close(client);
    close(s);
    connection_created = false;
    return 0;
}

/**
 * available: Get the number of available Packets.
 * @return Number of available Packets.
 */
size_t PHMS_Bluetooth::Server::available()
{
    return pkt_buffer.size();
}

/**
 * quit: Stops execution of the run() function.
 */
void PHMS_Bluetooth::Server::quit()
{
    is_quit = true;
}

/**
 * run: Thread entry point. Receives packets until quit() is called.
 */
void PHMS_Bluetooth::Server::run()
{
    if (connection_created == false)
        return;

    uint8_t buffer[MAX_PKT_SIZE]{0};

    while (is_quit == false)
    {
        memset(buffer, 0, MAX_PKT_SIZE);

        // read data from the client
        bytes_read = read(client, buffer, MAX_PKT_SIZE);
        if (bytes_read > 0)
        {
            // printf("received [%s]\n", buffer);
            pkt_guard.lock();
            PHMS_Bluetooth::Packet p(bytes_read, buffer);
            pkt_buffer.push_back(p);
            pkt_guard.unlock();
        }
    }
}

/**
 * get_all: Get a vector containing all received packets.
 * Clears the packet buffer.
 * TODO: This is super inefficient. Gets the job done I guess.
 */
std::vector<PHMS_Bluetooth::Packet> PHMS_Bluetooth::Server::get_all()
{
    pkt_guard.lock();
    std::vector<Packet> ret = pkt_buffer;
    pkt_buffer.clear();
    pkt_guard.unlock();
    return ret;
}