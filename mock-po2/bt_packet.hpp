#pragma once

#include <chrono>
#include <memory>
#include <cstring>

namespace Bluetooth_Connection
{
    class Packet
    {
    private:
        std::chrono::system_clock::time_point timestamp;
        size_t length;
        std::unique_ptr<uint8_t> data;

    public:
        Packet(size_t len, void *src);
    };

} // namespace Bluetooth_Connection
