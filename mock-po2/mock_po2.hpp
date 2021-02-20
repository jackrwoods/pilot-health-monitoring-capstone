#pragma once

#include <string>
#include <random>

#include "./bluetooth_con.hpp"

// defined constants are pulled from collected data

// MEAN_VALUE is the average value with full blood oxygen accounting for errors in measurement
// MEAN_RANGE is the general range that 'acceptable' values are inside of.
#define MEAN_VALUE 14000
#define MEAN_RANGE 250

// ~60% of measured PO2 values outside of the normal range and considered errors.
#define ERROR_RATE .60

// Max error value from recorded data.
#define POSITIVE_ERROR_MAX 65335

// Min error value from recorded data.
#define NEGATIVE_ERROR_MIN 55

// Minimum and maximum repetitions for good values
#define ACCEPTABLE_REPEAT_MAX 19
#define ACCEPTABLE_REPEAT_MIN 1
#define ACCEPTABLE_REPEAT_AVG 2.136

// Minimum and maximum positive error repeat. How many times do positive errors tend to occur in a row.
#define POSITIVE_ERROR_REPEAT_MAX 19
#define POSITIVE_ERROR_REPEAT_MIN 1
#define POSITIVE_ERROR_REPEAT_AVG 2.457

// Minimum and maximum negative error repeat. How many times do negative errors tend to occur in a row.
#define NEGATIVE_ERROR_REPEAT_MAX 3
#define NEGATIVE_ERROR_REPEAT_MIN 1
#define NEGATIVE_ERROR_REPEAT_AVG 1.196


// Do not allow compilation if ranges are bad
// positive error maximum must be greater than mean value
#if POSITIVE_ERROR_MAX < MEAN_VALUE
#error POSITIVE_ERROR_MAX must be defined to be greater than or equal to MEAN_VALUE
#endif

// negative error minimum must be smaller than mean value
#if NEGATIVE_ERROR_MAX > MEAN_VALUE
#error NEGATIVE_ERROR_MAX must be defined to be greater than or equal to MEAN_VALUE
#endif

// acceptable repetitions max value must be greater than acceptable repetitions min value
#if ACCEPTABLE_REPEAT_MAX < ACCEPTABLE_REPEAT_MIN
#error ACCEPTABLE_REPEAT_MAX must be greater than or equal to ACCEPTABLE_REPEAT_MIN
#endif

// positive error repeat maximum must be greater than positive error repeat minimum
#if POSITIVE_ERROR_REPEAT_MAX < POSITIVE_ERROR_REPEAT_MIN
#error POSITIVE_ERROR_REPEAT_MAX must be greater than or equal to POSITIVE_ERROR_REPEAT_MIN
#endif

// negative error repeat maximum must be greater than negative error repeat minimum
#if NEGATIVE_ERROR_REPEAT_MAX < NEGATIVE_ERROR_REPEAT_MIN
#error NEGATIVE_ERROR_REPEAT_MAX must be greater than or equal to NEGATIVE_ERROR_REPEAT_MIN
#endif

class Mock_PO2_sensor
{
private:
    PHMS_Bluetooth::Client client;
    std::string bt_address;

    uint32_t* data {nullptr};

    int good_repeat{0};
    int bad_positive_repeat{0};
    int bad_negative_repeat{0};


    uint32_t get_negative_bad();
    uint32_t get_positive_bad();
    uint32_t get_good();

public:
    Mock_PO2_sensor();
    ~Mock_PO2_sensor();
    uint32_t get();
};