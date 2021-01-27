#pragma once

#include <mutex>

#include "ds_types.hpp"
/**
 * Looping_Buffer
 * Constant buffer for reading and writing
 */
template <typename TYPE, int LENGTH>
class Looping_Buffer
{
private:
	T buffer[LENGTH];
	std::mutex mut;
	uint32_t count; // count the number of received samples
public:
	int block_read(int from, int to, TYPE *dest);
	int try_read(int from, int to, TYPE *dest);

	int block_write(TYPE *src, size_t len);
	int try_write(TYPE *src, size_t len);
};