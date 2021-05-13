#pragma once

#include <string>
#include <vector>
#include <cstdlib>

#include "./mock_po2.hpp"
#include "../data-server/include/datasource.hpp"
#include "../data-server/include/bluetooth_utils.hpp"

// add some variability into it
#define SENSOR_ERROR 200            // total error range, centered around SENSOR_ERROR/2
#define SENSOR_ERROR_PROBABILITY 70 // percent chance an error will occur

// get the same exact data for each sensor, variation is added in each Mock_Sensor object
class Inner_Sensor
{
private:
    Mock_PO2_Sensor po2_sensor;
    std::vector<Sample> stressed_buffer;
    int stressed_position{0};

    std::vector<Sample> unstressed_buffer;
    int unstressed_position{0};
    int stress_level;

public:
    Inner_Sensor(const std::string stressed_file, const std::string unstressed_file);

    void po2_adjust(int i)
    {
        po2_sensor.po2_adjust(i);
    }

    void inc_pos(int i);
    void set_stress_level(int stress);
    std::vector<Sample> get(int amount);
};

Inner_Sensor::Inner_Sensor(const std::string stressed_file, const std::string unstressed_file)
{
    stressed_buffer = sample_buffer_from_file(stressed_file);
    unstressed_buffer = sample_buffer_from_file(unstressed_file);

    if (stressed_buffer.size() == 0 || unstressed_buffer.size() == 0)
    {
        std::cerr << "(Inner Sensor) One or more input files resulted in an empty Sample buffer. Files: " << stressed_file << ", " << unstressed_file << std::endl;
        exit(1);
    }
    // seed random for Mock_Sensors. The Inner_Sensor object should exist before the Mock_Sensor objects
    srand(time(nullptr));
}

/**
  * inc_pos: increment the position of the current buffer in the inner sensor object
  * @param i: amount to increment position by
  */
void Inner_Sensor::inc_pos(int i)
{
    if (stress_level >= 5)
        stressed_position += i;
    else
        unstressed_position += i;

    stressed_position %= stressed_buffer.size();
    unstressed_position %= unstressed_buffer.size();
}

/**
  * set_stress_level: Set the stress level that the mock sensor should output
  * @param stress: a value between 0 and 10
  */
void Inner_Sensor::set_stress_level(int stress)
{
    if (stress >= 0 && stress <= 10)
    {
        // lower po2 with increased stress
        po2_sensor.po2_adjust(stress - stress_level);
        stress_level = stress;
    }
}

/**
  * get: Get a number of Samples, combination of simulation and recorded data
  * @param amount: the number of Samples to retrieve
  * @returns: a vector of the requested Samples
  */
std::vector<Sample> Inner_Sensor::get(int amount)
{
    // just in case check bounds
    if (amount + stressed_position > stressed_buffer.size() - 1)
        stressed_position = 0;

    if (amount + unstressed_position > unstressed_buffer.size() - 1)
        unstressed_position = 0;

    std::vector<Sample> ret;
    if (stress_level >= 5)
        ret = std::vector<Sample>(stressed_buffer.begin() + stressed_position, stressed_buffer.begin() + stressed_position + amount);
    else
        ret = std::vector<Sample>(unstressed_buffer.begin() + unstressed_position, unstressed_buffer.begin() + unstressed_position + amount);
    
    for(auto &r : ret)
        r.spo2 = po2_sensor.get();
        
    return ret;
}

/**
  * Mock_Sensor: Simulate values for a bpm and po2 sensor based on recorded and simulated values
  */
class Mock_Sensor
{
private:
    bool is_valid{true};

    Inner_Sensor *inner_sensor;

public:
    // Inner_Sensor generates the same data for each object,
    // Mock_Sensor adds some variability
    Mock_Sensor(Inner_Sensor *i) : inner_sensor(i){};

    std::vector<Sample> get(int amount);
    Sample get();

    void set_stress_level(int stress);

    void make_invalid();
    void make_valid();

    void inc_pos(int i);
};

/**
  * get: Get a number of Samples, combination of simulation and recorded data. If
  * the sensor has been set to invalid, random values will be produced
  * @param amount: the number of Samples to retrieve
  * @returns: a vector of the requested Samples
  */
std::vector<Sample> Mock_Sensor::get(int amount)
{
    std::vector<Sample> ret = inner_sensor->get(amount);
    if (is_valid)
    {
        // generate valid values add some variability to ret - revisit these values, they might be too random
        for (auto v = ret.begin(); v != ret.end(); v++)
        {
            if(rand() & 100 >= SENSOR_ERROR_PROBABILITY)
                v->irLED += (rand() % SENSOR_ERROR) - (SENSOR_ERROR / 2);

            if(rand() & 100 >= SENSOR_ERROR_PROBABILITY)
                v->redLED += (rand() % SENSOR_ERROR) - (SENSOR_ERROR / 2);
        }
    }
    else
    {
        // generate invalid values
        for (auto v = ret.begin(); v != ret.end(); v++)
        {
            v->irLED = rand() % UINT16_MAX;
            v->redLED = rand() % UINT16_MAX;
        }
    }
    return ret;
}

/**
  * set_stress_level: Set the stress level that the mock sensor should output
  * @param stress: a value between 0 and 10
  */
void Mock_Sensor::set_stress_level(int stress)
{
    inner_sensor->set_stress_level(stress);
}

/**
  * make_invalid: set the state of the sensor to invalid
  * this affects the values that are generated
  */
void Mock_Sensor::make_invalid()
{
    is_valid = false;
}

/**
  * make_valid: set the state of the sensor to valid
  * this affects the values that are generated
  */
void Mock_Sensor::make_valid()
{
    is_valid = true;
}

/**
  * inc_pos: increment the position of the current buffer in the inner sensor object
  * @param i: amount to increment position by
  */
void Mock_Sensor::inc_pos(int i)
{
    inner_sensor->inc_pos(i);
}