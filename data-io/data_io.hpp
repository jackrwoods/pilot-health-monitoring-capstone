#include <functional>

#include "./bluetooth/bt_client.hpp"
#include "./bluetooth/bt_server.hpp"
#include "./io_types.hpp"

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

template <class SAMPLE_TYPE>
void Data_IO<SAMPLE_TYPE>::run()
{
}

template <class SAMPLE_TYPE>
void Data_IO<SAMPLE_TYPE>::quit()
{
}

template <class SAMPLE_TYPE>
bool Data_IO<SAMPLE_TYPE>::register_callback(std::function<int(SAMPLE_TYPE *, size_t)> fn)
{
}

template <class SAMPLE_TYPE>
bool Data_IO<SAMPLE_TYPE>::connect(std::string bluetooth_address)
{
}

template <class SAMPLE_TYPE>
void Data_IO<SAMPLE_TYPE>::send_pilot_state(Pilot_State state)
{
}