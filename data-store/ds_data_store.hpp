#pragma once

#include <thread>
#include <vector>
#include <map>

#include "ds_types.hpp"
#include "ds_looping_buffer.hpp"

#define BUFFER_LENGTH 64

/**
 * Reader
 * Identifies a data reader using std::thread::id
 * Keeps track of reader's last read sample
 */
struct Reader
{
    std::vector<Sample> sample_buffer;
    uint32_t count{0};
};

Reader::Reader()
{
    sample_buffer.reserve(BUFFER_LENGTH);
}
/**
 * Data_Store
 * Hold all recorded biometric data.
 * Coordinate data access to other objects in other threads.
 */
template <typename LENGTH>
class Data_Store
{
private:
    std::map<std::thread::id, Reader> read_buffers;

    Looping_Buffer<Sample, BUFFER_LENGTH> samples;

    uint32_t bpm_variance;
    uint32_t bpm_average;
    uint32_t po2_average;

    uint32_t ece_bpm;
    uint32_t ece_po2;

    void sql_loop();

public:
    Data_Store();
    ~Data_Store();

    void set_bpm_variance();
    void set_bpm_average();
    void set_po2_average();
    void set_ece_bpm();
    void set_ece_po2();

    uint32_t get_bpm_variance();
    uint32_t get_bpm_average();
    uint32_t get_po2_average();
    uint32_t get_ece_bpm();
    uint32_t get_ece_po2();

    void new_data(Sample *src, size_t len);
    void new_data(Sample s);

    void register_reader_thread();

    std::vector<Sample>::iterator begin();
    std::vector<Sample>::iterator end();

    const std::vector<Sample>& vec();

    int copy(Sample* s, size_t len);

    int available_samples();
};