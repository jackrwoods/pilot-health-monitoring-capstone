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

        std::string get_server_connected_address() {return s.get_connected_address();}
        std::string get_client_connected_address() {return c.get_connected_address();}

        int open_con(std::string addr);
        int close_con();

        void run();
        void quit();

        // client functions
        void push(const Packet &p) {c.push(p);}
        void push(const void* src, size_t len) {c.push(src, len);}

        // server functions
        size_t available() {return s.available();}
        std::vector<Packet> get_all() {return s.get_all();}
    };
} // namespace PHMS_Bluetooth

/**
 * open_con: Creates a two way bluetooth connection.
 * @param addr The bluetooth address of device to connect to.
 * @returns 0 on successful client connection. Server connection is optional.
 */
int PHMS_Bluetooth::Communicator::open_con(std::string addr)
{
    // open the server in a temporary thread to accept a connection first
    std::thread server_connect(&PHMS_Bluetooth::Server::open_con, &s);
    // wait a little for other device to also start before requesting a connection
    usleep(5000);

    int con_status = c.open_con(addr);
    if(con_status != 0)
    {
        std::cerr << "(Bluetooth Communicator) an error occurred connecting to device at address " << addr << std::endl;
        return 1;
    }

    server_connect.join();
    return 0;
}

int PHMS_Bluetooth::Communicator::close_con()
{
    return s.close_con() + c.close_con();
}

void PHMS_Bluetooth::Communicator::run()
{
    std::cout << "(Bluetooth Communicator) spawning client and server run threads\n";
    server_thread = std::thread(&Server::run, &s);
    client_thread = std::thread(&Client::run, &c);
}

void PHMS_Bluetooth::Communicator::quit()
{
    s.quit();
    c.quit();

    server_thread.join();
    client_thread.join();
}