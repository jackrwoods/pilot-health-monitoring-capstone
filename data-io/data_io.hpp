#include <iostream>
#include <functional>
#include <thread>

#include "./bluetooth/bt_client.hpp"
#include "./bluetooth/bt_server.hpp"
#include "./io_types.hpp"

#include "../data-store/ds_data_store.hpp" // temporary until i find a better way to do this.

// identify what kind of data is in a packet
#define ID_SAMPLE 0xFF

#define ID_ECE_BPM 0xFE
#define ID_ECE_PO2 0xFD

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

    Data_Store<SAMPLE_TYPE, 256> &ds;

    // std::function<int(const SAMPLE_TYPE *, size_t)> insert_callback;

public:
    Data_IO(Data_Store<SAMPLE_TYPE, 256> &);
    ~Data_IO();

    void run();
    void quit();

    // bool register_callback(std::function<int(const SAMPLE_TYPE *, size_t)> fn);
    bool connect(const std::string bluetooth_address);

    void send_pilot_state(IO_TYPES::Pilot_State state);
};

template <class SAMPLE_TYPE>
Data_IO<SAMPLE_TYPE>::Data_IO(Data_Store<SAMPLE_TYPE, 256> &d) : ds(d)
{
    bt_server.open_con();

    bt_c = std::thread(&PHMS_Bluetooth::Client::run, std::ref(bt_client));
    bt_s = std::thread(&PHMS_Bluetooth::Server::run, std::ref(bt_server));

    ds.register_writer_thread();
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
            // receive packets
            auto packets = bt_server.get_all();
            for (auto i = packets.begin(); i != packets.end(); i++)
            {
                // std::cout << "got packet with id byte: " << i->get()[0] << std::endl;
                // printf("got packet with id byte: 0x%x\n", i->get()[0]);
                switch (i->get()[0])
                {
                // case ID_SAMPLE:
                case ID_SAMPLE:
                {
                    // printf("got sample packet(0x%x)\n", ID_SAMPLE);

                    // construct sample buffer from packet
                    int samples_len = i->size() / 4;

                    const uint32_t *data = reinterpret_cast<const uint32_t *>(i->get());

                    std::vector<IO_TYPES::Sample> smps;
                    for (int j = 0; j < samples_len; j += 2)
                    {
                        smps.push_back(IO_TYPES::Sample(data[1 + j], data[1 + j + 1]));
                    }
                    std::cout << ds.new_data(smps.data(), smps.size()) * 2 << ", ";
                    // for(auto j = smps.begin(); j != smps.end(); j++)
                        // std::cout << *j << std::endl;

                    break;
                }
                case ID_ECE_BPM:
                {
                    // printf("got ece bpm packet(0x%x) value: 0x%x\n", ID_ECE_BPM, reinterpret_cast<const uint32_t *>(i->get())[1]);
                    ds.set_ece_bpm(reinterpret_cast<const uint32_t *>(i->get())[1]);
                    break;
                }
                case ID_ECE_PO2:
                {
                    // printf("got ece po2 packet(0x%x) value: 0x%x\n", ID_ECE_PO2, reinterpret_cast<const uint32_t *>(i->get())[1]);
                    ds.set_ece_po2(reinterpret_cast<const uint32_t *>(i->get())[1]);
                    break;
                }
                default:
                    printf("got unrecognized packet(0x%x)\n", i->get()[0]);
                    break;
                }
            }
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
// template <class SAMPLE_TYPE>
// bool Data_IO<SAMPLE_TYPE>::register_callback(std::function<int(const SAMPLE_TYPE *, size_t)> fn)
// {
//     insert_callback = fn;
//     return true;
// }

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
void Data_IO<SAMPLE_TYPE>::send_pilot_state(IO_TYPES::Pilot_State state)
{
    if (bt_client.connection_created)
        bt_client.push(PHMS_Bluetooth::Packet(&state, sizeof(IO_TYPES::Pilot_State)));
    else
        std::cerr << "No active Bluetooth client connection.\n";
}