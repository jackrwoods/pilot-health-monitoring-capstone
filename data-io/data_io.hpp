#include <iostream>
#include <functional>
#include <thread>

#include "./bluetooth/bt_client.hpp"
#include "./bluetooth/bt_server.hpp"
#include "./io_types.hpp"

#define ID_STATE 0x1
#define ID_SAMPLE 0x2

#define ID_ECE_BPM 0x3
#define ID_ECE_PO2 0x4

/**
 * Data_IO: Receive Bluetooth data and write to datastore
 */
template <typename SAMPLE_TYPE>
class Data_IO
{
private:
    bool is_quit{false};

    // bluetooth communication
    PHMS_Bluetooth::Client bt_client;
    PHMS_Bluetooth::Server bt_server;

    // bluetooth threads
    std::thread bt_c;
    std::thread bt_s;

    std::function<int(SAMPLE_TYPE *, size_t)> insert_callback;

public:
    Data_IO();
    ~Data_IO();

    void run();
    void quit();

    bool register_callback(std::function<int(SAMPLE_TYPE *, size_t)> fn);
    bool connect(const std::string bluetooth_address);

    void send_pilot_state(Pilot_State state);
};

template <class SAMPLE_TYPE>
Data_IO<SAMPLE_TYPE>::Data_IO()
{
    bt_server.open_con();

    bt_c = std::thread(&PHMS_Bluetooth::Client::run, std::ref(bt_client));
    bt_s = std::thread(&PHMS_Bluetooth::Server::run, std::ref(bt_server));
}

template <class SAMPLE_TYPE>
Data_IO<SAMPLE_TYPE>::~Data_IO()
{
    bt_client.quit();
    
    bt_server.close_con();
    bt_server.quit();

    bt_c.join();
    bt_s.join();
}

/**
 * run: Begin the recieve loop.
 */
template <class SAMPLE_TYPE>
void Data_IO<SAMPLE_TYPE>::run()
{

    while (is_quit == false)
    {
        if (bt_server.available() > 0)
        {
            auto packets = bt_server.get_all();
            for (auto i = packets.begin(); i != packets.end(); i++)
                i->print();
        }
    }
}

/**
 * send_pilot_state: Transmit pilot state over Bluetooth connection
 * @param state: Pilot state to send.
 */
template <class SAMPLE_TYPE>
void Data_IO<SAMPLE_TYPE>::quit()
{
    is_quit = true;
    bt_server.quit();
    bt_client.quit();
}

/**
 * register_callback: Register a function to use whenever new data is available.
 * @param fn: Callback function
 * @returns: True if successful, false otherwise
 */
template <class SAMPLE_TYPE>
bool Data_IO<SAMPLE_TYPE>::register_callback(std::function<int(SAMPLE_TYPE *, size_t)> fn)
{
    insert_callback = fn;
    return true;
}

/**
 * connect: Initialize a Bluetooth connection.
 * @param bluetooth_address: Bluetooth address to connect to.
 * @returns: True if successful, false otherwise
 */
template <class SAMPLE_TYPE>
bool Data_IO<SAMPLE_TYPE>::connect(std::string bluetooth_address)
{
    return bt_client.open_con(bluetooth_address) == 0 ? true : false;
}

/**
 * send_pilot_state: Transmit pilot state over Bluetooth connection
 * @param state: Pilot state to send.
 */
template <class SAMPLE_TYPE>
void Data_IO<SAMPLE_TYPE>::send_pilot_state(Pilot_State state)
{
    if (bt_client.connection_created)
        bt_client.push(PHMS_Bluetooth::Packet(&state, 1));
    else
        std::cerr << "No active Bluetooth client connection.\n";
}