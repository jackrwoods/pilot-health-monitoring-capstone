#pragma once

#include <string>
#include <random>

// defined constants are pulled from collected data

// MEAN_VALUE is the average value with full blood oxygen accounting for errors in measurement
// MEAN_RANGE is the general range that 'acceptable' values are inside of.
#define MEAN_VALUE 14000
#define MEAN_RANGE 100

// ~60% of measured PO2 values outside of the normal range and considered errors.
// These need to add up to exactly 100
#define SUCCESS_RATE 40
#define NEGATIVE_ERROR_RATE 9
#define POSITIVE_ERROR_RATE 51

// Max error value from recorded data.
#define POSITIVE_ERROR_MAX 65335

// Min error value from recorded data.
#define NEGATIVE_ERROR_MIN 55

// Minimum and maximum repetitions for good values
#define ACCEPTABLE_REPEAT_MAX 19
#define ACCEPTABLE_REPEAT_MIN 5
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

enum gen_type
{
    ACCEPTABLE,
    POSITIVE_ERROR,
    NEGATIVE_ERROR
};

/**
 * Mock_PO2_Sensor: Generate values similar to those measured collected from actual po2 sensor.
 */
class Mock_PO2_Sensor
{
private:
    int good_repeat{0};
    int bad_positive_repeat{0};
    int bad_negative_repeat{0};

    uint32_t get_negative_bad();
    uint32_t get_positive_bad();
    uint32_t get_good();

    gen_type get_next_type();
    int in_range(int low, int high);

    int running_mean{MEAN_VALUE};

public:
    Mock_PO2_Sensor();
    ~Mock_PO2_Sensor();

    uint32_t get();
    std::string string_get();
    void po2_adjust(int);
};

Mock_PO2_Sensor::Mock_PO2_Sensor()
{
    srand(time(nullptr));

    // data = new uint32_t[samples];

    // for(int i = 0; i < samples; i++)
    // data[i] = get();
}

Mock_PO2_Sensor::~Mock_PO2_Sensor()
{
    // if (data != nullptr)
    // delete[] data;
}

/**
 * get_negative_bad: Generate a bad po2 value.
 * @return 32 bit bad po2 value below normal mean.
 */
uint32_t Mock_PO2_Sensor::get_negative_bad()
{
    ++bad_negative_repeat;
    bad_positive_repeat = 0;
    good_repeat = 0;
    return in_range(NEGATIVE_ERROR_MIN, running_mean - MEAN_RANGE);
}

/**
 * get_positive_bad: Generate a bad po2 value.
 * @return 32 bit bad po2 value above normal mean.
 */
uint32_t Mock_PO2_Sensor::get_positive_bad()
{
    ++bad_positive_repeat;
    bad_negative_repeat = 0;
    good_repeat = 0;
    return in_range(running_mean + MEAN_RANGE, POSITIVE_ERROR_MAX);
}

/**
 * get_good: Generate a good po2 value.
 * @return 32 bit good po2 value.
 */
uint32_t Mock_PO2_Sensor::get_good()
{
    ++good_repeat;
    bad_positive_repeat = 0;
    bad_negative_repeat = 0;
    return in_range(running_mean - MEAN_RANGE, running_mean + MEAN_RANGE);
}

/**
 * get: Get a generated po2 value.
 * @return A generated 32 bit po2 value.
 */
uint32_t Mock_PO2_Sensor::get()
{
    switch (get_next_type())
    {
    case gen_type::ACCEPTABLE:
        return get_good();
    case gen_type::POSITIVE_ERROR:
        return get_positive_bad();
    default:
        return get_negative_bad();
    }
}

/**
 * in_range: Get a random value.
 * @param low Lower bound of the random value.
 * @param high Upper bound of the random value.
 */
int Mock_PO2_Sensor::in_range(int low, int high)
{
    if (low == high)
        return low;

    if (low > high)
        return 0;

    return rand() % (high - low) + low;
}

/**
 * get_next_type: Calculate the next type (acceptable, positive error, negative error).
 * @return Next gen_type;
 */
gen_type Mock_PO2_Sensor::get_next_type()
{
    // probabilities based on defined constants
    int success = (good_repeat < ceil(ACCEPTABLE_REPEAT_AVG)) ? SUCCESS_RATE : SUCCESS_RATE / 2;
    int p_fail = (bad_positive_repeat < ceil(POSITIVE_ERROR_REPEAT_AVG)) ? POSITIVE_ERROR_RATE : POSITIVE_ERROR_RATE / 2;
    int n_fail = (bad_negative_repeat < ceil(NEGATIVE_ERROR_REPEAT_AVG)) ? POSITIVE_ERROR_RATE : POSITIVE_ERROR_RATE / 2;

    // No going over the maximum repititions
    if (good_repeat > ACCEPTABLE_REPEAT_MAX)
        success = 0;

    if (bad_positive_repeat > POSITIVE_ERROR_REPEAT_MAX || bad_negative_repeat > 0)
        p_fail = 0;

    if (bad_negative_repeat > NEGATIVE_ERROR_REPEAT_MAX || bad_positive_repeat > 0)
        n_fail = 0;

    // No going under the minimum repititions
    if (good_repeat > 0 && good_repeat < ACCEPTABLE_REPEAT_MIN)
        return gen_type::ACCEPTABLE;

    if (bad_positive_repeat > 0 && bad_positive_repeat < POSITIVE_ERROR_REPEAT_MIN)
        return gen_type::POSITIVE_ERROR;

    if (bad_negative_repeat > 0 && bad_negative_repeat < NEGATIVE_ERROR_REPEAT_MIN)
        return gen_type::NEGATIVE_ERROR;

    // Generate the correct value.
    int chance = in_range(0, success + p_fail + n_fail);

    if (0 <= chance && chance < n_fail)
        return gen_type::NEGATIVE_ERROR;

    if (n_fail <= chance && chance < n_fail + p_fail)
        return gen_type::POSITIVE_ERROR;

    // if(success + p_fail <= chance && chance <= success + p_fail + n_fail)
    // ACCEPTABLE will be the default
    return gen_type::ACCEPTABLE;
}

/**
 * po2_adjust: Adjust the generated po2 values.
 * @param adj Amount to adjust the running po2 average by.
 */
void Mock_PO2_Sensor::po2_adjust(int adj)
{
    running_mean += adj;
}

/**
 * string_get: Get a std::string with the value and the type.
 * @return std::string with generated value and type.
 */
std::string Mock_PO2_Sensor::string_get()
{
    uint32_t val = get();
    std::string s;

    if (good_repeat > 0)
        s = "(ACCEPTABLE)";
    else if (bad_positive_repeat > 0)
        s = "(POSITIVE_ERROR)";
    else if (bad_negative_repeat > 0)
        s = "(NEGATIVE_ERROR)";

    return std::to_string(val) + "\t" + s;
}