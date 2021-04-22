#pragma once

#include <chrono>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>

#include "./bluetooth/bluetooth_con.hpp"

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

enum UnitOfMeasure
{
    CELSIUS,
    MS,
    NONE
};

struct Value
{
    bool exists{false};
    UnitOfMeasure unit{NONE};
    double value{0.0L};
};

enum Source
{
    MAX30100,
    ECE_BLACK_BOX,
    BT_DATA_SENSOR,
    UNKNOWN
};

struct Sample
{
    // std::chrono::_V2::system_clock::time_point timestamp; // Timestamp corresponding to when this was created
    long timestamp;             // Timestamp corresponding to when this was created - ms since collection was started?
    Source sourceType{UNKNOWN}; // Describes what created this sample. ie: data interpreter, sensor, etc
    Value irLED;                // Infrared LED observation
    Value redLED;               // Red LED observation
    Value temperature;          // Temperature observation
    Value bpm;                  // Heart rate
    Value avg_bpm;              // Average heart rate
    Value spo2;                 // Blood oxygen content
    Value pilotState;           // The enum for Pilot State can be cast to a double
};

// conversions - defined here so that moving between types is easy

// definitely rework this before final checkoff
long from_time_stamp_string(std::string s)
{
    // 08:47:27.971
    // hours : minutes : seconds : milleseconds
    // assuming that hours and minutes are always two characters
    int hours = std::stoi(s.substr(0, 2));
    int minutes = std::stoi(s.substr(2, 2));
    int milliseconds = 1000 * std::stof(s.substr(6));

    long total_milliseconds = milliseconds + (60 * 60 * 1000 * hours) + (60 * 1000 * minutes);
    return total_milliseconds;
}

// construct a Sample based on a single string from the input csv file
// this function is super rigid and will only work if the csv is formatted exactly right
Sample from_string(std::string csv_string)
{
    Sample ret;
    std::array<std::string, 9> ar;
    int index{0};

    for (auto &i : ar)
    {
        int range_end = csv_string.find(',', index + 1);
        i = csv_string.substr(index + 1, range_end - index - 1);
        if (i == " ")
            i.clear();

        index = range_end;
    }

    // manually assign each member of the Sample

    ret.sourceType = BT_DATA_SENSOR;

    // timestamp [0]
    if (ar[0].size() > 0)
        ret.timestamp = from_time_stamp_string(ar[0]);

    // bpm [1]
    if (ar[1].size() > 0)
    {
        ret.bpm.exists = true;
        ret.bpm.value = std::stod(ar[1]);
    }
    // average bpm [2]
    if (ar[2].size() > 0)
    {
        ret.avg_bpm.exists = true;
        ret.avg_bpm.value = std::stod(ar[2]);
    }
    // red [3]
    if (ar[3].size() > 0)
    {
        ret.redLED.exists = true;
        ret.redLED.value = std::stod(ar[3]);
    }
    // ir [4]
    if (ar[4].size() > 0)
    {
        ret.irLED.exists = true;
        ret.irLED.value = std::stod(ar[4]);
    }

    // NO MEMBERS IN SAMPLE STRUCT FOR HEART RATE
    // HRvalid [6]
    // HR [5]
    // check HRvalid first
    // if(ar[6].size() > 0 && std::stoi(ar[6]) == 1 && ar[5].size() > 0)
    // {
    //     ret.?.exists = true;
    //     ret.?.value = std::stod(ar[5]);
    // }

    // SPO2Valid [8]
    // SPO2 [7]
    // check spo2valid first
    if (ar[8].size() > 0 && std::stoi(ar[8]) == 1 && ar[7].size() > 0)
    {
        ret.spo2.exists = true;
        ret.spo2.value = std::stod(ar[7]);
    }

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

    // sort samples by timestamp
    std::sort(ret.begin(), ret.end(), [](auto a, auto b) { return a.timestamp > b.timestamp; });

    return ret;
}

// read a double from a byte pointer, return the length of the double in bytes
int byte_to_double(const uint8_t *byte, Value &v)
{
    v.exists = true;
    v.value = *reinterpret_cast<const double *>(byte);
    return sizeof(v.value);
}

// pull samples from bluetooth packet
std::vector<Sample> sample_buffer_from_bt_packet(PHMS_Bluetooth::Packet p)
{
    std::vector<Sample> ret;
    Sample smp;

    const uint8_t *data = p.get();
    int bytes = p.size();
    int current_byte{0};

    while (current_byte < bytes)
    {
        // increment current byte past identifier
        // can begin by reading from current_byte as data
        switch (data[current_byte++])
        {
        case (BYTE_IDENTIFIER_timestamp):
            // this is a new sample - add to buffer and reset smp
            ret.push_back(smp);
            smp = Sample();

            smp.timestamp = *reinterpret_cast<const long *>(&data[current_byte]);
            current_byte += sizeof(smp.timestamp);
            break;

        case (BYTE_IDENTIFIER_sourceType):
            smp.sourceType = (Source)data[current_byte];
            current_byte += 1;
            break;

        case (BYTE_IDENTIFIER_irLED):
            current_byte += byte_to_double(&data[current_byte], smp.irLED);
            break;

        case (BYTE_IDENTIFIER_redLED):
            current_byte += byte_to_double(&data[current_byte], smp.redLED);
            break;

        case (BYTE_IDENTIFIER_temperature):
            current_byte += byte_to_double(&data[current_byte], smp.temperature);
            break;

        case (BYTE_IDENTIFIER_bpm):
            current_byte += byte_to_double(&data[current_byte], smp.bpm);
            break;

        case (BYTE_IDENTIFIER_avg_bpm):
            current_byte += byte_to_double(&data[current_byte], smp.avg_bpm);
            break;

        case (BYTE_IDENTIFIER_spo2):
            current_byte += byte_to_double(&data[current_byte], smp.spo2);
            break;

        case (BYTE_IDENTIFIER_pilotState):
            break;

        default:
            std::cerr << "unrecognized byte in packet:\n";
            p.print();
            exit(1);
        }
    }

    ret.erase(ret.begin(), ret.begin() + 1);
    ret.push_back(smp);

    return ret;
}

std::vector<uint8_t> double_to_bytes(double d)
{
    std::vector<uint8_t> ret;
    uint8_t *data = reinterpret_cast<uint8_t *>(&d);
    for (int i = 0; i < sizeof(d); i++)
        ret.push_back(data[i]);
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

// construct bluetooth packet from sample buffer
PHMS_Bluetooth::Packet packet_from_Sample_buffer(const std::vector<Sample> &samples)
{
    std::vector<uint8_t> bytes;
    std::vector<uint8_t> add;

    for (const auto &i : samples)
    {
        // timestamp
        bytes.push_back(BYTE_IDENTIFIER_timestamp);
        add = long_to_bytes(i.timestamp);
        bytes.insert(bytes.end(), add.begin(), add.end());

        // sourceType
        bytes.push_back(BYTE_IDENTIFIER_sourceType);
        bytes.push_back((uint8_t)i.sourceType);

        // irLED
        if (i.irLED.exists)
        {
            bytes.push_back(BYTE_IDENTIFIER_irLED);
            add = double_to_bytes(i.irLED.value);
            bytes.insert(bytes.end(), add.begin(), add.end());
        }

        // redLED
        if (i.redLED.exists)
        {
            bytes.push_back(BYTE_IDENTIFIER_redLED);
            add = double_to_bytes(i.redLED.value);
            bytes.insert(bytes.end(), add.begin(), add.end());
        }

        // temperature
        if (i.temperature.exists)
        {
            bytes.push_back(BYTE_IDENTIFIER_temperature);
            add = double_to_bytes(i.temperature.value);
            bytes.insert(bytes.end(), add.begin(), add.end());
        }

        // bpm
        if (i.bpm.exists)
        {
            bytes.push_back(BYTE_IDENTIFIER_bpm);
            add = double_to_bytes(i.bpm.value);
            bytes.insert(bytes.end(), add.begin(), add.end());
        }

        // avg_bpm
        if (i.avg_bpm.exists)
        {
            bytes.push_back(BYTE_IDENTIFIER_avg_bpm);
            add = double_to_bytes(i.avg_bpm.value);
            bytes.insert(bytes.end(), add.begin(), add.end());
        }
        // spo2
        if (i.spo2.exists)
        {
            bytes.push_back(BYTE_IDENTIFIER_spo2);
            add = double_to_bytes(i.spo2.value);
            bytes.insert(bytes.end(), add.begin(), add.end());
        }

        // pilotState
        if (i.pilotState.exists)
        {
            bytes.push_back(BYTE_IDENTIFIER_pilotState);
            add = double_to_bytes(i.pilotState.value);
            bytes.insert(bytes.end(), add.begin(), add.end());
        }
    }

    return PHMS_Bluetooth::Packet(bytes.size(), bytes.data());
}

// print all information from a sample
void print(const Sample &sample)
{
    // timestamp
    printf("\nSample at %i\n", sample.timestamp);

    // source
    printf("   Source: ");
    switch (sample.sourceType)
    {
    case (MAX30100):
        printf("MAX30100\n");
        break;
    case (ECE_BLACK_BOX):
        printf("ECE BLACK BOX\n");
        break;
    case (BT_DATA_SENSOR):
        printf("BT_DATA_SENSOR\n");
        break;
    default:
        printf("UNKNOWN\n");
        break;
    }

    // irLED
    printf("   irLED : ");
    (sample.irLED.exists) ? printf("%f\n", sample.irLED.value) : printf("No value\n");

    // redLED
    printf("   redLED : ");
    (sample.redLED.exists) ? printf("%f\n", sample.redLED.value) : printf("No value\n");

    // temperature
    printf("   temperature: ");
    (sample.temperature.exists) ? printf("%f\n", sample.temperature.value) : printf("No value\n");

    // bpm
    printf("   bpm: ");
    (sample.bpm.exists) ? printf("%f\n", sample.bpm.value) : printf("No value\n");

    // average bpm
    printf("   avg_bpm: ");
    (sample.avg_bpm.exists) ? printf("%f\n", sample.avg_bpm.value) : printf("No value\n");

    // spo2
    printf("   spo2: ");
    (sample.spo2.exists) ? printf("%f\n", sample.spo2.value) : printf("No value\n");

    // pilot state
    printf("   pilot state: ");
    (sample.pilotState.exists) ? printf("%f\n", sample.pilotState.value) : printf("No value\n");
}

void print_small(const Sample &sample)
{
    printf("sample at %i\n", sample.timestamp);
}