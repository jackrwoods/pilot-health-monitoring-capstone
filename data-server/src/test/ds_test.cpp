#include <iostream>
#include <assert.h>

#include "ds_looping_buffer.hpp"
#include "ds_data_store.hpp"

#define SAMPLE_COUNT 2048

void thread_write(Data_Store<Sample> *ds_p, Sample *s)
{
    Data_Store<Sample> &ds = *ds_p;

    // measure time to add data
    auto start = std::chrono::high_resolution_clock::now();
    ds.new_data(s, 16);
    auto end = std::chrono::high_resolution_clock::now();

    auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() / 64;
    float hz = 1000000000.0 / nanos;
    std::cout << "Copied 16 items in " << nanos << " nanoseconds (max data transfer: " << hz << " hz)" << std::endl;
}

void thread_ex(Data_Store<Sample> *ds_p, Sample *s)
{

    Data_Store<Sample> &ds = *ds_p;

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

void parallel_thread_write(Data_Store<Sample> *ds_p, Sample *s, int sample_count)
{
    Data_Store<Sample> &ds = *ds_p;

    // feed the datastore 32 Samples, wait a little while after - will still be faster than 64hz
    for(int i = 0; i < SAMPLE_COUNT; i += 32)
    {
        if(i > 0 && s[0].optical != 69)
            s[0] = Sample(69, 69);

        assert(ds.new_data(s + i, 32) == 32);
        printf("> WRITE added %d samples to the data store\n", 32);

        // wait
        std::this_thread::sleep_for(std::chrono::milliseconds(400));
    }
    printf("> WRITE finished writing %d samples to the data store\n", sample_count);
}

void parallel_thread_read(Data_Store<Sample> *ds_p, Sample *s, int sample_count)
{
    Data_Store<Sample> &ds = *ds_p;
    ds.register_reader_thread();

    int recv_samples {0};
    while(recv_samples != sample_count)
    {
        if(ds.available_samples() > 0)
        {
            int count {0};
            for(auto i = ds.begin(); i != ds.end(); i++)
            {
                ++count;

                // assert that the values being received are what is expected
                assert(*i == s[recv_samples]);
                recv_samples++;
            }
            std::cout << "> READ " << std::this_thread::get_id() << " read " << count << " samples from the data store\n";
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100 + rand()%500));
    }
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

    Data_Store<Sample> ds;

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

    std::cout << "\n=== Data Validity Test ===\n";

    // array of samples for comparison
    Sample s[SAMPLE_COUNT];

    // populate sample array
    for (int i = 0; i < SAMPLE_COUNT; i++)
    {
        s[i] = Sample(static_cast<po2_sample>(i), static_cast<optical_sample>(i));
    }

    // multithreaded read tests
    // writer will write

    std::thread th_writer(parallel_thread_write, &ds, s, SAMPLE_COUNT);

    std::vector<std::thread> read_threads;
    for(int i = 0; i < 8; i++)
        read_threads.push_back(std::thread(parallel_thread_read, &ds, s, SAMPLE_COUNT));

    th_writer.join();
    
    for(auto &i : read_threads)
        i.join();

    std::cout << "\nPassed!" << std::endl;
}

int main()
{
    srand(time(nullptr));
    
    test_lb();
    test_ds();
    std::cout << "All tests passed" << std::endl;

    return 0;
}