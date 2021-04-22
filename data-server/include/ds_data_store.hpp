#pragma once

#include <thread>
#include <vector>
#include <unordered_map>

#include "datasource.hpp"
#include "ds_looping_buffer.hpp"

#define BUFFER_LENGTH 1024

/**
 * Reader
 * Identifies a data reader using std::thread::id
 * Keeps track of reader's last read sample
 */
template <typename SAMPLE_TYPE>
struct Reader
{
    std::vector<SAMPLE_TYPE> sample_buffer;
    uint32_t count{0};
    Reader();
};

template <class SAMPLE_TYPE>
Reader<SAMPLE_TYPE>::Reader()
{
    sample_buffer.reserve(BUFFER_LENGTH);
}

/**
 * Data_Store
 * Hold all recorded biometric data.
 * Coordinate data access to other objects in other threads.
 * @param SAMPLE_TYPE Type of sample to hold
 * @param LENGTH Number of samples to hold
 */
template <typename SAMPLE_TYPE>
class Data_Store
{
private:
    std::unordered_map<std::thread::id, Reader<SAMPLE_TYPE>> read_buffers;
    std::mutex map_guard;

    std::thread::id writer{0};
    bool writer_registered{false};

    Looping_Buffer<SAMPLE_TYPE, BUFFER_LENGTH> samples;

    uint32_t bpm_variance{0};
    uint32_t bpm_average{0};
    uint32_t po2_average{0};

    uint32_t ece_bpm{0};
    uint32_t ece_po2{0};

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

    int new_data(SAMPLE_TYPE *src, size_t len);
    void new_data(SAMPLE_TYPE *s);
    int new_data(SAMPLE_TYPE s);

    void register_reader_thread();

    typename std::vector<SAMPLE_TYPE>::iterator begin();
    typename std::vector<SAMPLE_TYPE>::iterator end();

    const std::vector<SAMPLE_TYPE> &vec();

    int copy(SAMPLE_TYPE *s, size_t len);

    int available_samples();
    int size();
};

template <typename SAMPLE_TYPE>
Data_Store<SAMPLE_TYPE>::Data_Store(Datasource* ds)
{
    // reserve space for 16 potential reader threads
    read_buffers.reserve(16);

	// Listen to the datasource for new data asynchronously
	std::function<void(SAMPLE_TYPE*)> callback(std::bind(&Data_Store::new_data, this));
	ds->registerCallback(callback);
}

template <typename SAMPLE_TYPE>
Data_Store<SAMPLE_TYPE>::~Data_Store()
{
}

/**
 * set_ece_po2: Set CS team calculated BPM variance
 */
template <typename SAMPLE_TYPE>
void Data_Store<SAMPLE_TYPE>::set_bpm_variance(uint32_t i)
{
    bpm_variance = i;
}

/**
 * set_ece_po2: Set CS team calculated running average BPM value
 */
template <typename SAMPLE_TYPE>
void Data_Store<SAMPLE_TYPE>::set_bpm_average(uint32_t i)
{
    bpm_average = i;
}

/**
 * set_ece_po2: Set CS team calculated running average PO2 value
 */
template <typename SAMPLE_TYPE>
void Data_Store<SAMPLE_TYPE>::set_po2_average(uint32_t i)
{
    po2_average = i;
}

/**
 * set_ece_po2: Set ECE team calculated BPM value
 */
template <typename SAMPLE_TYPE>
void Data_Store<SAMPLE_TYPE>::set_ece_bpm(uint32_t i)
{
    ece_bpm = i;
}

/**
 * set_ece_po2: Set ECE team calculated PO2 value
 */
template <typename SAMPLE_TYPE>
void Data_Store<SAMPLE_TYPE>::set_ece_po2(uint32_t i)
{
    ece_po2 = i;
}

/**
 * get_bpm_variance: Get CS team calculated BPM variance
 * @returns CS team calculated BPM variance
 */
template <typename SAMPLE_TYPE>
uint32_t Data_Store<SAMPLE_TYPE>::get_bpm_variance() const
{
    return bpm_variance;
}

/**
 * get_bpm_average: Get CS team calculated BPM running average value
 * @returns CS team calculated BPM running average value
 */
template <typename SAMPLE_TYPE>
uint32_t Data_Store<SAMPLE_TYPE>::get_bpm_average() const
{
    return bpm_average;
}

/**
 * get_po2_average: Get CS team calculated PO2 running average value
 * @returns CS team calculated PO2 running average value
 */
template <typename SAMPLE_TYPE>
uint32_t Data_Store<SAMPLE_TYPE>::get_po2_average() const
{
    return po2_average;
}

/**
 * get_ece_bpm: Get ECE team calculated BPM value
 * @returns ECE team calculated BPM value
 */
template <typename SAMPLE_TYPE>
uint32_t Data_Store<SAMPLE_TYPE>::get_ece_bpm() const
{
    return ece_bpm;
}

/**
 * get_ece_po2: Get ECE team calculated PO2 value
 * @returns ECE team calculated PO2 value
 */
template <typename SAMPLE_TYPE>
uint32_t Data_Store<SAMPLE_TYPE>::get_ece_po2() const
{
    return ece_po2;
}

/**
 * new_data: Add one or more Samples to the data buffer. Calling thread must be registered as writer thread.
 * @param src: Pointer to beginning of SAMPLE_TYPE buffer to be added
 * @param len: Number of SAMPLE_TYPE to be added
 * @returns Number of SAMPLE_TYPE successfully added to the buffer
 */
template <typename SAMPLE_TYPE>
int Data_Store<SAMPLE_TYPE>::new_data(SAMPLE_TYPE *src, size_t len)
{
    return samples.block_write(src, len);
}

/**
 * new_data: Add one SAMPLE_TYPE to the data buffer. Calling thread must be registered as writer thread.
 * @param s: SAMPLE_TYPE to add
 * @returns Number of SAMPLE_TYPE successfully added to buffer
 */
template <typename SAMPLE_TYPE>
int Data_Store<SAMPLE_TYPE>::new_data(SAMPLE_TYPE s)
{
    return samples.block_write(&s, 1);
}

/**
 * new_data: Add one SAMPLE_TYPE to the data buffer. Calling thread must be registered as writer thread.
 * @param s: SAMPLE_TYPE to add
 * @returns Number of SAMPLE_TYPE successfully added to buffer
 */
template <typename SAMPLE_TYPE>
void Data_Store<SAMPLE_TYPE>::new_data(SAMPLE_TYPE *s)
{
	return samples.block_write(s, 1);
}


/**
 * register_reader_thread: Register current thread as a reader thread
 */
template <typename SAMPLE_TYPE>
void Data_Store<SAMPLE_TYPE>::register_reader_thread()
{
    // read_buffers[std::thread::id];
    std::lock_guard<std::mutex> guard(map_guard);
    read_buffers.insert(std::make_pair(std::this_thread::get_id(), Reader<SAMPLE_TYPE>()));
}

/**
 * begin: Copy newest available samples into vector, return iterator.
 * @returns Vector iterator at beginning of new samples vector
 */
template <typename SAMPLE_TYPE>
typename std::vector<SAMPLE_TYPE>::iterator Data_Store<SAMPLE_TYPE>::begin()
{
    apply_new_data(std::this_thread::get_id());
    return read_buffers.at(std::this_thread::get_id()).sample_buffer.begin();
}

/**
 * end: Iterator at end of samples vector
 * @returns vector iterator at end of samples vector
 */
template <typename SAMPLE_TYPE>
typename std::vector<SAMPLE_TYPE>::iterator Data_Store<SAMPLE_TYPE>::end()
{
    return read_buffers.at(std::this_thread::get_id()).sample_buffer.end();
}

/**
 * vec: Get a reference to the vector of new available samples
 * @returns Vector reference
 */
template <typename SAMPLE_TYPE>
const std::vector<SAMPLE_TYPE> &Data_Store<SAMPLE_TYPE>::vec()
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
template <typename SAMPLE_TYPE>
int Data_Store<SAMPLE_TYPE>::copy(SAMPLE_TYPE *s, size_t len)
{
    return samples.copy_to(s, samples.samples_recv() - len, samples.samples_recv());
}

/**
 * available_samples: How many unread samples are available to each thread
 * @returns New samples available to thread
 */
template <typename SAMPLE_TYPE>
int Data_Store<SAMPLE_TYPE>::available_samples()
{
    return samples.samples_recv() - read_buffers.at(std::this_thread::get_id()).count;
}

/**
 * apply_new_data: Internal function for copying data into thread buffers.
 * @param id Thread id
 */
template <typename SAMPLE_TYPE>
void Data_Store<SAMPLE_TYPE>::apply_new_data(const std::thread::id id)
{
    Reader<SAMPLE_TYPE> &reader = read_buffers.at(id);

    // copy directly into vector
    // this is probably a bad idea but I
    // could not think of a better way to do this

    reader.sample_buffer.clear();
    reader.sample_buffer.resize(available_samples());

    reader.count += samples.copy_to(&reader.sample_buffer[0], reader.count, samples.samples_recv());
    // add exceptions if missed data?
}

template <class SAMPLE_TYPE>
int Data_Store<SAMPLE_TYPE>::size()
{
    return BUFFER_LENGTH;
}
