#pragma once

#include <mutex>
#include <cstring>

#include "ds_types.hpp"
/**
 * Looping_Buffer
 * Constant buffer for reading and writing
 * @param TYPE data type of buffer
 * @param LENGTH number of data samples to store in buffer
 */
template <typename TYPE, int LENGTH>
class Looping_Buffer
{
private:
	TYPE buffer[LENGTH];
	std::mutex mut; // control access to the buffer
	uint32_t count; // count the number of received samples
public:
	Looping_Buffer();

	int block_read(int from, int to, TYPE *dest);
	int try_read(int from, int to, TYPE *dest);

	int block_write(const TYPE *src, size_t len);
	int try_write(const TYPE *src, size_t len);

	void print_state();
};

template <class TYPE, int LENGTH>
Looping_Buffer<TYPE, LENGTH>::Looping_Buffer()
{
	memset(buffer, 0, sizeof(TYPE) * LENGTH);
}

/**
 * block_read: Unlock buffer and copy data from the buffer. 
 * Block if buffer is locked.
 * @param from Beginning sample
 * @param to Ending sample
 * @param dest Location to copy samples to
 * @returns Number of samples successfully read
 */
template <class TYPE, int LENGTH>
int Looping_Buffer<TYPE, LENGTH>::block_read(int from, int to, TYPE *dest)
{
	return 0;
}

/**
 * try_read: attempt to unlock the buffer and copy data from 
 * the buffer. Return if unable to lock.
 * @param from Beginning sample
 * @param to Ending sample
 * @param dest Location to copy samples to
 * @returns Number of samples successfully read
 */
template <class TYPE, int LENGTH>
int Looping_Buffer<TYPE, LENGTH>::try_read(int from, int to, TYPE *dest)
{
	return 0;
}

/**
 * block_write: write to the buffer from src. Block if buffer is unavailable.
 * @param src data source
 * @param len how many samples to copy
 * @returns Number of samples successfully read
 */
template <class TYPE, int LENGTH>
int Looping_Buffer<TYPE, LENGTH>::block_write(const TYPE *src, size_t len)
{
	return 0;
}

/**
 * try_write: write to the buffer from src. Return if buffer is unavailable.
 * @param src data source
 * @param len how many samples to copy
 * @returns Number of samples successfully read
 */
template <class TYPE, int LENGTH>
int Looping_Buffer<TYPE, LENGTH>::try_write(const TYPE *src, size_t len)
{
	return 0;
}

/**
 * print_state: print the current contents and position of looping buffer
 */
template <class TYPE, int LENGTH>
void Looping_Buffer<TYPE, LENGTH>::print_state()
{
	mut.lock();
	for (int i = 0; i < LENGTH; i++)
	{
		if (i == count % LENGTH)
			printf("> %i (%i)\n", buffer[i], i);
		else
			printf("  %i\n", buffer[i]);
	}
	mut.unlock();
}