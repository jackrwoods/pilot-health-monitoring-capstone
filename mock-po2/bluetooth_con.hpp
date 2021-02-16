#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

#include <random>

#define MAX_TRANSMIT_SIZE 64
#define MIN_TRANSMIT_SIZE 32

class Bluetooth_Connection
{
private:
    int average{0};
    int variance{0};

    uint32_t data_buffer[MAX_TRANSMIT_SIZE];

public:
    Bluetooth_Connection();
    Bluetooth_Connection(std::string MAC);
    ~Bluetooth_Connection();

    bool connect(std::string MAC);
    bool close();

    int adjust_po2(int i);
};

Bluetooth_Connection::Bluetooth_Connection()
{
}

Bluetooth_Connection::Bluetooth_Connection(std::string MAC)
{
}

Bluetooth_Connection::~Bluetooth_Connection()
{
}

bool Bluetooth_Connection::connect(std::string MAC)
{
}

bool Bluetooth_Connection::close()
{
}

int Bluetooth_Connection::adjust_po2(int i)
{
}