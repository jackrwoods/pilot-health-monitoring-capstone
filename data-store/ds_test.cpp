#include <iostream>
#include <assert.h>

#include "ds_looping_buffer.hpp"

int main()
{
    std::cout << "Data_Store tests" << std::endl;

    Looping_Buffer<uint32_t, 16> lb;

    uint32_t buffer_0[16] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    uint32_t buffer_1[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    // block_write() writes all 16 uint32_t to looping buffer
    assert(16 == lb.block_write(buffer_0, 16));

    // block_read() reads all 16 uint32_t to buffer_1
    assert(16 == lb.block_read(0, 16, buffer_1));

    // all 16 uint32_t are written from buffer_0 to buffer_1
    for(int i = 0; i < 16; i++) {
        assert(buffer_0[i] == buffer_1[i]);
        // for the next test;
        buffer_0[i] *= i;
    }

    // block_write() writes all 16 uint32_t to looping buffer
    assert(16 == lb.block_write(buffer_0, 16));

    // block_read() reads all 16 uint32_t to buffer_1
    assert(16 == lb.block_read(0, 16, buffer_1));

    // all 16 uint32_t are written from buffer_0 to buffer_1
    for(int i = 0; i < 16; i++) {
        assert(buffer_0[i] == buffer_1[i]);
    }

    std::cout << "All tests passed" << std::endl;

    return 0;
}