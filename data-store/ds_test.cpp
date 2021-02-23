#include <iostream>
#include <assert.h>

#include "ds_looping_buffer.hpp"
#include "ds_data_store.hpp"

#define BUFFER_LEN 64

void thread_write(Data_Store<Sample, BUFFER_LEN> *ds_p, Sample *s)
{
    Data_Store<Sample, BUFFER_LEN> &ds = *ds_p;

    // register thread as writer
    ds.register_writer_thread();

    // measure time to add data
    auto start = std::chrono::high_resolution_clock::now();
    ds.new_data(s, 16);
    auto end = std::chrono::high_resolution_clock::now();

    auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() / 64;
    float hz = 1000000000.0 / nanos;
    std::cout << "Copied 16 items in " << nanos << " nanoseconds (max data transfer: " << hz << " hz)" << std::endl;
}

void thread_ex(Data_Store<Sample, BUFFER_LEN> *ds_p, Sample *s)
{

    Data_Store<Sample, BUFFER_LEN> &ds = *ds_p;

    // register as reader thread
    ds.register_reader_thread();

    // begin clock
    auto start = std::chrono::high_resolution_clock::now();

    // receive personal vector with data
    const std::vector<Sample> &v = ds.vec();

    // all 16 Samples should be in vector
    assert(v.size() == 16);

    // assert that samples read into vector are identical to those written to vector from Sample *s
    for (int i = 0; i < 16; i++)
    {
        assert(v[i] == s[i]);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() / 16;
    float hz = 1000000000.0 / nanos;
    std::cout << "Thread " << std::this_thread::get_id() << " read and processed 16 Samples in " << nanos << " nanoseconds (max data transfer: " << hz << " hz)\n";
}

void test_lb()
{
    std::cout << "looping buffer tests: ";

    Looping_Buffer<uint32_t, 16> lb;

    uint32_t buffer_0[16] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    uint32_t buffer_1[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    // block_write() writes all 16 uint32_t to looping buffer
    assert(16 == lb.block_write(buffer_0, 16));

    // block_read() reads all 16 uint32_t to buffer_1
    assert(16 == lb.block_read(0, 16, buffer_1));

    // all 16 uint32_t are written from buffer_0 to buffer_1
    for (int i = 0; i < 16; i++)
    {
        assert(buffer_0[i] == buffer_1[i]);
        // for the next test;
        buffer_0[i] *= i;
    }

    // block_write() writes all 16 uint32_t to looping buffer
    assert(16 == lb.block_write(buffer_0, 16));

    // block_read() reads all 16 uint32_t to buffer_1
    assert(16 == lb.block_read(0, 16, buffer_1));

    // all 16 uint32_t are written from buffer_0 to buffer_1
    for (int i = 0; i < 16; i++)
    {
        assert(buffer_0[i] == buffer_1[i]);
    }

    // try_write() writes all 8 uint32_t to looping buffer
    assert(8 == lb.block_write(buffer_0, 8));

    // try_read() reads all 8 uint32_t to buffer_1
    assert(8 == lb.block_read(32, 40, buffer_1));

    for (int i = 0; i < 8; i++)
    {
        // printf("%i:%i\n", buffer_0[i], buffer_1[i]);
        assert(buffer_0[i] == buffer_1[i]);
    }

    // block_read() fails when asked to copy more than LENGTH items
    assert(0 == lb.block_read(0, 40, buffer_0));

    // block_read() fails when asked to copy negative items
    assert(0 == lb.block_read(40, 0, buffer_0));

    // block_read() fails when asked to copy ahead of current most recent sample
    assert(0 == lb.block_read(40, 50, buffer_0));

    // accurate samples received
    assert(40 == lb.samples_recv());

    std::cout << "Passed!" << std::endl;
}
void test_ds()
{
    std::cout << "Data_Store tests: ";

    Data_Store<Sample, BUFFER_LEN> ds;

    std::cout << "\n=== Measure time to set values ===\n";

    // measure time to set values
    auto ts_start = std::chrono::high_resolution_clock::now();
    ds.set_bpm_average(7);
    auto ts_end = std::chrono::high_resolution_clock::now();
    auto ts_nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(ts_end - ts_start).count() / 64;
    float ts_hz = 1000000000.0 / ts_nanos;
    std::cout << "Set average bpm in " << ts_nanos << " nanoseconds (" << ts_hz << " hz)" << std::endl;

    ts_start = std::chrono::high_resolution_clock::now();
    ds.set_bpm_variance(7);
    ts_end = std::chrono::high_resolution_clock::now();
    ts_nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(ts_end - ts_start).count() / 64;
    ts_hz = 1000000000.0 / ts_nanos;
    std::cout << "Set bpm variance in " << ts_nanos << " nanoseconds (" << ts_hz << " hz)" << std::endl;

    ts_start = std::chrono::high_resolution_clock::now();
    ds.set_po2_average(7);
    ts_end = std::chrono::high_resolution_clock::now();
    ts_nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(ts_end - ts_start).count() / 64;
    ts_hz = 1000000000.0 / ts_nanos;
    std::cout << "Set po2 average in " << ts_nanos << " nanoseconds (" << ts_hz << " hz)" << std::endl;

    ts_start = std::chrono::high_resolution_clock::now();
    ds.set_ece_bpm(7);
    ts_end = std::chrono::high_resolution_clock::now();
    ts_nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(ts_end - ts_start).count() / 64;
    ts_hz = 1000000000.0 / ts_nanos;
    std::cout << "Set ece bpm in " << ts_nanos << " nanoseconds (" << ts_hz << " hz)" << std::endl;

    ts_start = std::chrono::high_resolution_clock::now();
    ds.set_ece_po2(7);
    ts_end = std::chrono::high_resolution_clock::now();
    ts_nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(ts_end - ts_start).count() / 64;
    ts_hz = 1000000000.0 / ts_nanos;
    std::cout << "Set ece po2 in " << ts_nanos << " nanoseconds (" << ts_hz << " hz)" << std::endl;

    // assert values were set correctly
    assert(ds.get_bpm_average() == 7);
    assert(ds.get_bpm_variance() == 7);
    assert(ds.get_po2_average() == 7);
    assert(ds.get_ece_bpm() == 7);
    assert(ds.get_ece_po2() == 7);

    std::cout << "\n=== Measure data transfer ===\n";

    // array of samples for comparison
    Sample s[16];

    // populate sample array
    for (int i = 0; i < 16; i++)
    {
        s[i] = Sample(static_cast<po2_sample>(i), static_cast<optical_sample>(i));
    }

    // read new data into data store - new thread for writer
    std::thread th_writer(thread_write, &ds, s);
    th_writer.join();

    // multithreaded read tests

    // thread_ex(&ds, s);

    std::thread th_0(thread_ex, &ds, s);
    std::thread th_1(thread_ex, &ds, s);
    std::thread th_2(thread_ex, &ds, s);
    std::thread th_3(thread_ex, &ds, s);
    std::thread th_4(thread_ex, &ds, s);

    th_0.join();
    th_1.join();
    th_2.join();
    th_3.join();
    th_4.join();
    std::cout << "\nPassed!" << std::endl;
}

int main()
{
    test_lb();
    test_ds();
    std::cout << "All tests passed" << std::endl;

    return 0;
}