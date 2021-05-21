#pragma once

#include <chrono>
#include <memory>
#include <cstring>

namespace PHMS_Bluetooth
{
    /*
    * Packet: Timestamped packet for arbitrary data
    */
    class Packet
    {
    private:
        std::chrono::system_clock::time_point timestamp;
        // size_t length{0};
        // std::unique_ptr<uint8_t> data;
        // uint8_t *data{nullptr};
        std::vector<uint8_t> data;

        Packet();

    public:
        Packet(size_t len, const void *src);
        Packet(const Packet &p);
        Packet(Packet &&p);

        ~Packet();

        size_t size();
        std::chrono::system_clock::time_point time();
        const uint8_t *get();

        void print();
    };
} // namespace PHMS_Bluetooth

PHMS_Bluetooth::Packet::Packet()
{
    timestamp = std::chrono::system_clock::now();
}

/**
 * Copy constructor
 * @param p: Bluetooth packet with data to be copied into current packet
 */
PHMS_Bluetooth::Packet::Packet(const PHMS_Bluetooth::Packet &p)
{
    timestamp = p.timestamp;
    data = p.data;
}

/**
 * Move constructor
 * @param p: Bluetooth packet with data to be moved into current packet.
 */
PHMS_Bluetooth::Packet::Packet(PHMS_Bluetooth::Packet &&p)
{
    timestamp = p.timestamp;
    data = std::move(p.data);
}

/**
 * Packet: Create a packet
 * @param len Size of the new packet in bytes
 * @param src Location of the data to construct the packet from
 */
PHMS_Bluetooth::Packet::Packet(size_t len, const void *src)
{
    timestamp = std::chrono::system_clock::now();
    data = std::vector<uint8_t>(reinterpret_cast<const uint8_t *>(src),reinterpret_cast<const uint8_t *>(src) + len);
}

// default destructor
PHMS_Bluetooth::Packet::~Packet()
{
}

/**
 * size: Return the size in bytes of the packet.
 */
size_t PHMS_Bluetooth::Packet::size()
{
    return data.size();
}

/**
 * time: Return the time the packet was recevied.
 */
std::chrono::system_clock::time_point PHMS_Bluetooth::Packet::time()
{
    return timestamp;
}

/**
 * get: Return a pointer to the data held inside the packet.
 */
const uint8_t *PHMS_Bluetooth::Packet::get()
{
    return &data.front();
}

/**
 * print: Print the contents of the packet
 */
void PHMS_Bluetooth::Packet::print()
{
    printf("== PACKET ============================\n");
    printf("= timestamp: %12u            =\n", timestamp.time_since_epoch());
    printf("= length: %4i                       =\n", size());
    printf("======================================");
    for (int i = 0; i < (size() / 8) + 1; i++)
    {
        printf("\n= ");
        // hex
        for (int j = 0; j < 8; j++)
        {
            if ((i * 8) + j < size())
                printf("%.2x ", data[(i * 8) + j]);
            else
                printf("   ");
        }

        printf("| ");

        // char
        for (int j = 0; j < 8; j++)
        {
            if ((i * 8) + j < size())
                (data[(i * 8) + j] >= 33 && data[(i * 8) + j] <= 126) ? printf("%c", data[(i * 8) + j]) : printf(".");
            else
                printf(" ");
        }

        printf(" =");
    }
    printf("\n======================================\n");
}