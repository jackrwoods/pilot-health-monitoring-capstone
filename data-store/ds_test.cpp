#include <iostream>
#include <assert.h>

#include "ds_looping_buffer.hpp"
#include "ds_data_store.hpp"

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

    // Data_Store<uint32_t> ds;
}
int main()
{
    Sample s;
    s.optical = 9;
    s.po2 = 6;
    std::cout << "Sample Struct COUT >> " << s << std::endl;
    test_lb();
    test_ds();
    std::cout << "All tests passed" << std::endl;

    return 0;
}