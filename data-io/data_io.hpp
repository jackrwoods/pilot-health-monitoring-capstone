#include <functional>

#include "./bluetooth/bt_client.hpp"
#include "./bluetooth/bt_server.hpp"
#include "./io_types.hpp"

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

    std::function<int(SAMPLE_TYPE *, size_t)> insert_callback;

public:
    void run();
    void quit();

    bool register_callback(std::function<int(SAMPLE_TYPE *, size_t)> fn);
    bool connect(const std::string bluetooth_address);

    void send_pilot_state(Pilot_State state);
};

/**
 * run: Begin the recieve loop.
 */
template <class SAMPLE_TYPE>
void Data_IO<SAMPLE_TYPE>::run()
{
}

/**
 * send_pilot_state: Transmit pilot state over Bluetooth connection
 * @param state: Pilot state to send.
 */
template <class SAMPLE_TYPE>
void Data_IO<SAMPLE_TYPE>::quit()
{
}

/**
 * register_callback: Register a function to use whenever new data is available.
 * @param fn: Callback function
 * @returns: True if successful, false otherwise
 */
template <class SAMPLE_TYPE>
bool Data_IO<SAMPLE_TYPE>::register_callback(std::function<int(SAMPLE_TYPE *, size_t)> fn)
{
}

/**
 * connect: Initialize a Bluetooth connection.
 * @param bluetooth_address: Bluetooth address to connect to.
 * @returns: True if successful, false otherwise
 */
template <class SAMPLE_TYPE>
bool Data_IO<SAMPLE_TYPE>::connect(std::string bluetooth_address)
{
}

/**
 * send_pilot_state: Transmit pilot state over Bluetooth connection
 * @param state: Pilot state to send.
 */
template <class SAMPLE_TYPE>
void Data_IO<SAMPLE_TYPE>::send_pilot_state(Pilot_State state)
{
}