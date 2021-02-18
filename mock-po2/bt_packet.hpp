#pragma once

#include <chrono>
#include <memory>
#include <cstring>

namespace Bluetooth_Connection
{
    /*
    * Packet: Timestamped packet for arbitrary data
    */
    class Packet
    {
    private:
        std::chrono::system_clock::time_point timestamp;
        size_t length{0};
        // std::unique_ptr<uint8_t> data;
        uint8_t *data{nullptr};

        Packet();

    public:
        Packet(size_t len, const void *src);
        Packet(const Packet &p);

        ~Packet();

        size_t size();
        std::chrono::system_clock::time_point time();
        const uint8_t *get();

        void print();
    };
} // namespace Bluetooth_Connection

Bluetooth_Connection::Packet::Packet()
{
    timestamp = std::chrono::system_clock::now();
}

/**
 * Copy constructor
 * TODO: this should probably be a move constructor. I can't figure it out, though...
 */
Bluetooth_Connection::Packet::Packet(const Bluetooth_Connection::Packet &p)
{
    length = p.length;
    timestamp = p.timestamp;

    data = new uint8_t[length];
    memcpy(data, p.data, length);
}

    /**
 * Packet: Create a packet
 * @param len Size of the new packet in bytes
 * @param src Location of the data to construct the packet from
 */
    Bluetooth_Connection::Packet::Packet(size_t len, const void *src)
    {
        timestamp = std::chrono::system_clock::now();
        data = new uint8_t[len];
        length = len;
        memcpy(data, src, len);
    }

    Bluetooth_Connection::Packet::~Packet()
    {
        if (data != nullptr)
            delete[] data;
    }

    /**
 * size: Return the size in bytes of the packet.
 */
    size_t Bluetooth_Connection::Packet::size()
    {
        return length;
    }

    /**
 * time: Return the time the packet was recevied.
 */
    std::chrono::system_clock::time_point Bluetooth_Connection::Packet::time()
    {
        return timestamp;
    }

    /**
 * get: Return a pointer to the data held inside the packet.
 */
    const uint8_t *Bluetooth_Connection::Packet::get()
    {
        return data;
    }

    /**
 * print: Print the contents of the packet
 */
    void Bluetooth_Connection::Packet::print()
    {
        printf("== PACKET ===================\n");
        printf("= timestamp: %u\n", timestamp.time_since_epoch());
        printf("= length: %i\n", length);
        printf("= data:");
        for (int i = 0; i < length; i++)
        {
            if (i % 8 == 0)
            {
                printf("\n= ");
            }
            printf("%.2x ", data[i]);
        }
        printf("\n=============================\n");
    }