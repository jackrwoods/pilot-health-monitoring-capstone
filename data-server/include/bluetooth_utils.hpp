#pragma once

#include <chrono>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <tuple>

#include "./bluetooth/bluetooth_con.hpp"
#include "./datasource.hpp"

// how to identify a field within the bluetooth packet
#define BYTE_IDENTIFIER_timestamp 0x01
#define BYTE_IDENTIFIER_sourceType 0x02
#define BYTE_IDENTIFIER_irLED 0x03
#define BYTE_IDENTIFIER_redLED 0x04
#define BYTE_IDENTIFIER_temperature 0x05
#define BYTE_IDENTIFIER_bpm 0x06
#define BYTE_IDENTIFIER_avg_bpm 0x07
#define BYTE_IDENTIFIER_spo2 0x08
#define BYTE_IDENTIFIER_pilotState 0x09

// construct a Sample based on a single string from the input csv file
// this function is super rigid and will only work if the csv is formatted exactly right
Sample from_string(std::string csv_string)
{
    // ir_led \t red_led \t spo2
    Sample ret;
    if (csv_string.empty())
        return ret;

    std::stringstream ss(csv_string);

    std::vector<std::string> sections;

    std::string s;
    while (std::getline(ss, s, '\t'))
    {
        sections.push_back(s);
    }

    ret.irLED = std::stoi(sections[0]);
    ret.redLED = std::stoi(sections[1]);
    ret.spo2 = std::stoi(sections[2]);

    return ret;
}

// create a buffer of samples sorted based on collection time
std::vector<Sample> sample_buffer_from_file(const std::string filename)
{
    std::vector<Sample> ret;

    std::ifstream f(filename, std::ios::in);
    if (!f.is_open())
        return ret;

    // skip first line
    std::string s;
    std::getline(f, s);

    while (!f.eof())
    {
        std::getline(f, s);
        ret.push_back(from_string(s));
    }

    return ret;
}

// quick way to package together the two results
struct Smp_with_Source
{
    std::vector<Sample> samples;
    uint8_t src;
};

inline uint16_t combine(uint8_t a, uint8_t b)
{
    return b + (a << 8);
}

// pull samples from bluetooth packet
Smp_with_Source sample_buffer_from_bt_packet(PHMS_Bluetooth::Packet p)
{
    Smp_with_Source ret;
    const uint8_t *bytes = p.get();
    ret.src = bytes[0];

    int cur_byte{1};

    unsigned long time = p.time().time_since_epoch().count();
    // useconds in 64 hz
    constexpr unsigned long time_interval = 1000000 / 64;
    int sample_count{1};

    while (cur_byte != p.size())
    {
        Sample s;
        s.irLED = combine(bytes[cur_byte], bytes[cur_byte + 1]);
        s.redLED = combine(bytes[cur_byte + 2], bytes[cur_byte + 3]);
        s.spo2 = combine(bytes[cur_byte + 4], bytes[cur_byte + 5]);
        cur_byte += 6;

        // time received and 1/64 of a second for each sample
        s.timestamp = time + (sample_count * time_interval);

        ret.samples.push_back(s);
        
        sample_count++;
    }

    return ret;
}

std::vector<uint8_t> long_to_bytes(long l)
{
    std::vector<uint8_t> ret;
    uint8_t *data = reinterpret_cast<uint8_t *>(&l);
    for (int i = 0; i < sizeof(l); i++)
        ret.push_back(data[i]);
    return ret;
}

void write_to_bytes(uint16_t t, std::vector<uint8_t> &v)
{
    uint8_t *data = reinterpret_cast<uint8_t *>(&t);
    v.push_back(data[1]);
    v.push_back(data[0]);
}

// construct bluetooth packet from sample buffer
PHMS_Bluetooth::Packet packet_from_Sample_buffer(uint8_t source_sensor, const std::vector<Sample> &samples)
{
    // add sensor source on initialization
    std::vector<uint8_t> bytes({source_sensor});

    // add sample data
    for (auto s : samples)
    {
        write_to_bytes(s.irLED, bytes);
        write_to_bytes(s.redLED, bytes);
        write_to_bytes(s.spo2, bytes);
    }

    return PHMS_Bluetooth::Packet(bytes.size(), &bytes.front());
}

// print all information from a sample
void print(const Sample &sample)
{
    printf("Sample ir LED: %x, red LED: %x, spo2: %x\n", sample.irLED, sample.redLED, sample.spo2);
}