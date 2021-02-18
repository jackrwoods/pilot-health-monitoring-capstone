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
        Server();
        Server(std::string addr);
        ~Server();

        int open_con(std::string addr);
        int close_con();

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
    open_con(addr);
}

/**
 * Default destructor
 */
Bluetooth_Connection::Server::~Server()
{
    close_con();
}

/**
 * open: Creates a bluetooth connection.
 * @param addr The bluetooth address of device to connect to.
 * @returns 0 on success.
 */
int Bluetooth_Connection::Server::open_con(std::string addr)
{
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
    if(client != -1)
        connection_created = true;

    return client;
}

/**
 * close: Closes existing bluetooth connection.
 * @return 0 on success.
 */
int Bluetooth_Connection::Server::close_con()
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
size_t Bluetooth_Connection::Server::available()
{
    return pkt_buffer.size();
}

/**
 * quit: Stops execution of the run() function.
 */
void Bluetooth_Connection::Server::quit()
{
    is_quit = true;
}

/**
 * run: Thread entry point. Receives packets until quit() is called.
 */
void Bluetooth_Connection::Server::run()
{
    if (connection_created == false)
        return;

    uint8_t buffer[MAX_PKT_SIZE] {0};

    while (is_quit == false)
    {
        ba2str(&rem_addr.l2_bdaddr, reinterpret_cast<char *>(buffer));
        fprintf(stderr, "accepted connection from %s\n", buffer);

        memset(buffer, 0, MAX_PKT_SIZE);

        // read data from the client
        bytes_read = read(client, buffer, MAX_PKT_SIZE);
        if (bytes_read > 0)
        {
            printf("received [%s]\n", buffer);
        }
    }
}