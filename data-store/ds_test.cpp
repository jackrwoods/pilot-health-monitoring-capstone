#include <iostream>
#include <assert.h>

#include "ds_looping_buffer.hpp"
#include "ds_data_store.hpp"

void thread_ex(Data_Store<Sample> *ds_p, Sample *s)
{

    Data_Store<Sample> &ds = *ds_p;

    // register as reader thread
    ds.register_reader_thread();

    // receive personal vector with data
    const std::vector<Sample> &v = ds.vec();

    // all 16 Samples should be in vector
    assert(v.size() == 16);

    // assert that samples read into vector are identical to those written to vector from Sample *s
    for (int i = 0; i < 16; i++)
    {
        assert(v[i] == s[i]);
    }
}

void test_lb()
{
    std::cout << "looping buffer tests:" << std::endl;

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

    lb.print_state();
}
void test_ds()
{
    std::cout << "Data_Store tests:" << std::endl;

    Data_Store<Sample> ds;

    // test gets and sets - should be pretty straightforward
    ds.set_bpm_average(7);
    ds.set_bpm_variance(7);
    ds.set_po2_average(7);
    ds.set_ece_bpm(7);
    ds.set_ece_po2(7);

    assert(ds.get_bpm_average() == 7);
    assert(ds.get_bpm_variance() == 7);
    assert(ds.get_po2_average() == 7);
    assert(ds.get_ece_bpm() == 7);
    assert(ds.get_ece_po2() == 7);

    // array of samples for comparison
    Sample s[16];

    // populate sample array
    for (int i = 0; i < 16; i++)
    {
        s[i] = Sample(static_cast<po2_sample>(i), static_cast<optical_sample>(i));
    }

    // read new data into data store
    ds.new_data(s, 16);

    // multithreaded tests
    std::thread th_0(thread_ex, &ds, s);
    std::thread th_1(thread_ex, &ds, s);

    th_0.join();
    th_1.join();
}

int main()
{
    test_lb();
    test_ds();
    std::cout << "All tests passed" << std::endl;

    return 0;
}