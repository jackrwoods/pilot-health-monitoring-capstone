#pragma once

#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>

// requires library libbluetooth-dev to be installed
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

#include <random>
#include <vector>

#include "./bt_client.hpp"
#include "./bt_server.hpp"
#include "./bt_packet.hpp"

// https://people.csail.mit.edu/albert/bluez-intro/x559.html
// uses l2cap sockets?

/**
 * Bluetooth_Connection
 * Create a read/write connection with a bluetooth device.
 */
namespace Bluetooth_Connection
{

    /**
     * Connection: Struct for packing multiple connection possibilities together.
     */
    struct Possible_Connection
    {
        char address[19]{0};
        char name[248]{0};
    };

    std::vector<Possible_Connection> available_connections();

    class Connection
    {
    private:
        Client client;
        Server server;

    public:
        Connection(std::string addr);
        ~Connection();

        bool open_connection(std::string addr);
        bool close_connection();

        bool active();

        size_t send(Packet &&p);
        size_t receive(void *dest, size_t max_len);
    };

} // namespace Bluetooth_Connection

Bluetooth_Connection::Connection::Connection(std::string addr)
{
    open_connection(addr);
}

Bluetooth_Connection::Connection::~Connection()
{
}

/**
 * open_connection: Creates a bluetooth connection.
 * @param addr the bluetooth address of device to connect to.
 * @returns True if a connection was created, false otherwise.
 */
bool Bluetooth_Connection::Connection::open_connection(std::string addr)
{
    return false;
}

/**
 * close_connection: Closes any open bluetooth connections.
 * @returns True if a connection was closed, false otherwise.
 */
bool Bluetooth_Connection::Connection::close_connection()
{
    return false;
}

/**
 * active: Is there an active bluetooth connection.
 * @returns True if there is an active bluetooth connection, false otherwise.
 */
bool Bluetooth_Connection::Connection::active()
{
    return false;
}

/**
 * available_connections: Construct and return a vector of available bluetooth connections.
 * @returns std::vector<Connection> listing all available bluetooth connections.
 */
std::vector<Bluetooth_Connection::Possible_Connection> Bluetooth_Connection::available_connections()
{
    // most of this code was adapted(stolen) from: https://people.csail.mit.edu/albert/bluez-intro/c404.html

    std::vector<Possible_Connection> cons;

    inquiry_info *ii{nullptr};
    int max_rsp, num_rsp;
    int dev_id, sock, len, flags;
    int i;

    dev_id = hci_get_route(nullptr);
    sock = hci_open_dev(dev_id);
    if (dev_id < 0 || sock < 0)
    {
        std::cerr << "An error occurred opening the bluetooth socket." << std::endl;
        return cons;
    }

    len = 8;
    max_rsp = 255;
    flags = IREQ_CACHE_FLUSH;
    ii = (inquiry_info *)malloc(max_rsp * sizeof(inquiry_info));

    num_rsp = hci_inquiry(dev_id, len, max_rsp, nullptr, &ii, flags);
    if (num_rsp < 0)
        std::cerr << "The hci_inquiry function encountered an error." << std::endl;

    for (i = 0; i < num_rsp; i++)
    {
        cons.push_back(Possible_Connection());
        ba2str(&(ii + i)->bdaddr, cons.back().address);
        memset(cons.back().name, 0, sizeof(cons.back().name));
        if (hci_read_remote_name(sock, &(ii + i)->bdaddr, sizeof(cons.back().name), cons.back().name, 0) < 0)
            strcpy(cons.back().name, "[unknown]");
    }

    free(ii);
    close(sock);

    return cons;
}