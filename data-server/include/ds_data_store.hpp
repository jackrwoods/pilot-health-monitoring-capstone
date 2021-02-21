#pragma once

#include <thread>
#include <vector>
#include <map>

#include "datasource.hpp"
#include "ds_looping_buffer.hpp"
#include "sql_con.hpp"

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
    Reader();
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

    uint32_t bpm_variance{0};
    uint32_t bpm_average{0};
    uint32_t po2_average{0};

    uint32_t ece_bpm{0};
    uint32_t ece_po2{0};

    SQL_Connection sql;

    void apply_new_data(const std::thread::id id);

public:
    Data_Store(Datasource* ds);
    ~Data_Store();

    void set_bpm_variance(uint32_t i);
    void set_bpm_average(uint32_t i);
    void set_po2_average(uint32_t i);
    void set_ece_bpm(uint32_t i);
    void set_ece_po2(uint32_t i);

    uint32_t get_bpm_variance() const;
    uint32_t get_bpm_average() const;
    uint32_t get_po2_average() const;
    uint32_t get_ece_bpm() const;
    uint32_t get_ece_po2() const;

    int new_data(Sample *s);

    void register_reader_thread();

    std::vector<Sample>::iterator begin();
    std::vector<Sample>::iterator end();

    const std::vector<Sample> &vec();

    int copy(Sample *s, size_t len);

    int available_samples();
};

template <class LENGTH>
Data_Store<LENGTH>::Data_Store(Datasource* ds) {
    	// Listen to the datasource for new data asynchronously
		std::function<void(struct Sample*)> callback(std::bind(&Data_Store::new_data, this, std::placeholders::_1));
		ds->registerCallback(callback);
}

template <class LENGTH>
Data_Store<LENGTH>::~Data_Store()
{
}

/**
 * set_ece_po2: Set CS team calculated BPM variance
 */
template <class LENGTH>
void Data_Store<LENGTH>::set_bpm_variance(uint32_t i)
{
    bpm_variance = i;
}

/**
 * set_ece_po2: Set CS team calculated running average BPM value
 */
template <class LENGTH>
void Data_Store<LENGTH>::set_bpm_average(uint32_t i)
{
    bpm_average = i;
}

/**
 * set_ece_po2: Set CS team calculated running average PO2 value
 */
template <class LENGTH>
void Data_Store<LENGTH>::set_po2_average(uint32_t i)
{
    po2_average = i;
}

/**
 * set_ece_po2: Set ECE team calculated BPM value
 */
template <class LENGTH>
void Data_Store<LENGTH>::set_ece_bpm(uint32_t i)
{
    ece_bpm = i;
}

/**
 * set_ece_po2: Set ECE team calculated PO2 value
 */
template <class LENGTH>
void Data_Store<LENGTH>::set_ece_po2(uint32_t i)
{
    ece_po2 = i;
}

/**
 * get_bpm_variance: Get CS team calculated BPM variance
 * @returns CS team calculated BPM variance
 */
template <class LENGTH>
uint32_t Data_Store<LENGTH>::get_bpm_variance() const
{
    return bpm_variance;
}

/**
 * get_bpm_average: Get CS team calculated BPM running average value
 * @returns CS team calculated BPM running average value
 */
template <class LENGTH>
uint32_t Data_Store<LENGTH>::get_bpm_average() const
{
    return bpm_average;
}

/**
 * get_po2_average: Get CS team calculated PO2 running average value
 * @returns CS team calculated PO2 running average value
 */
template <class LENGTH>
uint32_t Data_Store<LENGTH>::get_po2_average() const
{
    return po2_average;
}

/**
 * get_ece_bpm: Get ECE team calculated BPM value
 * @returns ECE team calculated BPM value
 */
template <class LENGTH>
uint32_t Data_Store<LENGTH>::get_ece_bpm() const
{
    return ece_bpm;
}

/**
 * get_ece_po2: Get ECE team calculated PO2 value
 * @returns ECE team calculated PO2 value
 */
template <class LENGTH>
uint32_t Data_Store<LENGTH>::get_ece_po2() const
{
    return ece_po2;
}

/**
 * new_data: Add one Sample to the data buffer
 * @param s: Sample to add
 * @returns Number of Samples successfully added to buffer
 */
template <class LENGTH>
int Data_Store<LENGTH>::new_data(Sample *s)
{
    return samples.block_write(s, 1);
}

/**
 * register_reader_thread: Register current thread as a reader thread
 */
template <class LENGTH>
void Data_Store<LENGTH>::register_reader_thread()
{
    // read_buffers[std::thread::id];
    read_buffers.insert(std::make_pair(std::this_thread::get_id(), Reader()));
}

/**
 * begin: Copy newest available samples into vector, return iterator.
 * @returns Vector iterator at beginning of new samples vector
 */
template <class LENGTH>
std::vector<Sample>::iterator Data_Store<LENGTH>::begin()
{
    apply_new_data(std::this_thread::get_id());
    return read_buffers.at(std::this_thread::get_id()).sample_buffer.begin();
}

/**
 * end: Iterator at end of samples vector
 * @returns vector iterator at end of samples vector
 */
template <class LENGTH>
std::vector<Sample>::iterator Data_Store<LENGTH>::end()
{
    return read_buffers.at(std::this_thread::get_id()).sample_buffer.end();
}

/**
 * vec: Get a reference to the vector of new available samples
 * @returns Vector reference
 */
template <class LENGTH>
const std::vector<Sample> &Data_Store<LENGTH>::vec()
{
    apply_new_data(std::this_thread::get_id());
    return read_buffers.at(std::this_thread::get_id()).sample_buffer;
}

/**
 * copy: Skip thread tracking, grab from oldest available Sample
 * @param s Location to copy samples to
 * @param len Number of samples to copy
 * @returns Number of samples successfully copied
 */
template <class LENGTH>
int Data_Store<LENGTH>::copy(Sample *s, size_t len)
{
    return samples.copy_to(s, samples.samples_recv() - len, samples.samples_recv());
}

/**
 * available_samples: How many unread samples are available to each thread
 * @returns New samples available to thread
 */
template <class LENGTH>
int Data_Store<LENGTH>::available_samples()
{
    return samples.samples_recv() - read_buffers.at(std::this_thread::get_id()).count;
}

/**
 * apply_new_data: Internal function for copying data into thread buffers.
 * @param id Thread id
 */
template <class LENGTH>
void Data_Store<LENGTH>::apply_new_data(const std::thread::id id)
{
    Reader &reader = read_buffers.at(id);

    // copy directly into vector
    // this is probably a bad idea but I 
    // could not think of a better way to do this

    reader.sample_buffer.clear();
    reader.sample_buffer.resize(samples.samples_recv() - reader.count);

    reader.count += samples.copy_to(&reader.sample_buffer[0], reader.count, samples.samples_recv());
    // add exceptions if missed data?
}