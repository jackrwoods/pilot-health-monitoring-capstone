#pragma once

#include <thread>

#include "bt_server.hpp"
#include "bt_client.hpp"

namespace PHMS_Bluetooth
{
    class Communicator
    {
    private:
        Server s;
        Client c;

        std::thread server_thread;
        std::thread client_thread;

    public:
        std::string get_server_connected_address() { return s.get_connected_address(); }
        std::string get_client_connected_address() { return c.get_connected_address(); }

        int open_con(std::string addr, int tries);
        int close_con();

        void run();
        void quit();

        // client functions
        void push(const Packet &p) { c.push(p); }
        void push(const void *src, size_t len) { c.push(src, len); }

        // server functions
        size_t available() { return s.available(); }
        std::vector<Packet> get_all() { return s.get_all(); }
    };
} // namespace PHMS_Bluetooth

/**
 * open_con: Creates a two way bluetooth connection.
 * @param addr The bluetooth address of device to connect to.
 * @param tries (optional) Number of times to attempt the connection until success. -1 to block until successful connection is created (or until integer underflow).
 * @returns 0 if successful two way connection was created.
 */
int PHMS_Bluetooth::Communicator::open_con(std::string addr, int tries = 1)
{

    // open the server connection creation method in a temporary thread to accept a connection first
    // run tries amount of times, a timeout is implemented in s.open_con()
    int s_tries = tries;
    std::thread server_connect([&]() {
        while (s.open_con() && s_tries--) {}
    });

    int con_status{1};
    do
    {
        con_status = c.open_con(addr);
        if (con_status == 0)
            break;

        // if connection is unsuccessful, wait for a little before trying again (0.5 s)
        usleep(500000);
    } while (con_status != 0 && tries-- != 0);

    // join server connection thread from earlier
    server_connect.join();

    if (con_status != 0 || s.get_connected_address().empty())
    {
        std::cerr << "(Bluetooth Communicator) an error occurred connecting to device at address " << addr << std::endl;
        return -1;
    }
    return 0;
}

/**
 * close_con: Closes existing bluetooth connections.
 * @return 0 on success.
 */
int PHMS_Bluetooth::Communicator::close_con()
{
    return s.close_con() + c.close_con();
}

/**
 * run: Spawn client and server threads
 */
void PHMS_Bluetooth::Communicator::run()
{
    std::cout << "(Bluetooth Communicator) spawning client and server run threads\n";
    server_thread = std::thread(&Server::run, &s);
    client_thread = std::thread(&Client::run, &c);
}

/**
 * quit: stop receiving and sending over Bluetooth. Join server and client threads
 */
void PHMS_Bluetooth::Communicator::quit()
{
    s.quit();
    c.quit();

    server_thread.join();
    client_thread.join();
}