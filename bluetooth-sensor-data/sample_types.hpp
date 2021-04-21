#pragma once

#include <chrono>
#include "./bluetooth/bluetooth_con.hpp"

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
    std::chrono::_V2::system_clock::time_point timestamp; // Timestamp corresponding to when this was created
    Source sourceType{UNKNOWN};                           // Describes what created this sample. ie: data interpreter, sensor, etc
    Value irLED;                                          // Infrared LED observation
    Value redLED;                                         // Red LED observation
    Value temperature;                                    // Temperature observation
    Value bpm;                                            // Heart rate
    Value spo2;                                           // Blood oxygen content
    Value pilotState;                                     // The enum for Pilot State can be cast to a double
};

// conversions - defined here so that moving between types is easy
