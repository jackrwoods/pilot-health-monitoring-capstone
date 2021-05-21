#pragma once

#include <mutex>
#include <cstring>

#include "datasource.hpp"
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
	std::mutex mut;	   // control access to the buffer
	uint32_t count{0}; // count the number of received samples

public:
	int copy_from(const TYPE *src, size_t len);
	int copy_to(TYPE *dest, int from, int to);
	Looping_Buffer();

	int block_read(int from, int to, TYPE *dest);
	int try_read(int from, int to, TYPE *dest);

	int block_write(const TYPE *src, size_t len);
	int try_write(const TYPE *src, size_t len);

	void print_state();

	int samples_recv();
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
	int read_count{0};
	mut.lock();
	// copy data
	read_count = copy_to(dest, from, to);
	mut.unlock();
	return read_count;
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
	int read_count{0};
	if (mut.try_lock())
	{
		// copy data
		read_count = copy_to(dest, from, to);
		mut.unlock();
	}
	return read_count;
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
	int written_count{0};
	mut.lock();
	// copy data
	written_count = copy_from(src, len);
	mut.unlock();
	count += written_count;
	return written_count;
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
	int written_count{0};
	if (mut.try_lock())
	{
		// copy data
		written_count = copy_from(src, len);
		mut.unlock();
		count += written_count;
	}
	return written_count;
}

/**
 * print_state: print the current contents and position of looping buffer
 */
template <class TYPE, int LENGTH>
void Looping_Buffer<TYPE, LENGTH>::print_state()
{
	// assumes that TYPE is an integral type
	printf("------------ print_state() ------------\n");
	mut.lock();
	for (int i = 0; i < LENGTH; i++)
	{
		if (i == count % LENGTH)
			printf("|> %i (%i)\n", buffer[i], count);
		else
			printf("|  %i\n", buffer[i]);
	}
	mut.unlock();
	printf("---------------------------------------\n");
}

/**
 * copy_from: Internal copying function. Copies len samples from buffer to src.
 * @param src data source
 * @param len how many bytes to copy
 * @returns Number of samples successfully read
 */
template <class TYPE, int LENGTH>
int Looping_Buffer<TYPE, LENGTH>::copy_from(const TYPE *src, size_t len)
{
	int items_copied{0};
	// do not copy more than LENGTH items:
	if (len > LENGTH)
	{
		return 0;
	}

	// detect when copy needs to be done twice
	if (len + count % LENGTH > LENGTH)
	{
		// split into two memcpy operations
		// define section beginnings
		TYPE *sec_0 = buffer + count % LENGTH;
		TYPE *sec_1 = buffer;

		// define section lengths
		int sec_0_len = LENGTH - count % LENGTH;
		int sec_1_len = len - sec_0_len;

		// copy data
		memcpy(sec_0, src, sec_0_len * sizeof(TYPE));
		memcpy(sec_1, src + sec_0_len, sec_1_len * sizeof(TYPE));

		items_copied += sec_0_len;
		items_copied += sec_1_len;
	}
	else
	{
		// can be read into contiguous memory
		memcpy(buffer + count % LENGTH, src, len * sizeof(TYPE));
		items_copied += len;
	}
	return items_copied;
}

/**
 * copy_to: Internal copying function. Copies len samples from dest to buffer.
 * @param dest data source
 * @param len how many bytes to copy
 * @returns Number of samples successfully read
 */
template <class TYPE, int LENGTH>
int Looping_Buffer<TYPE, LENGTH>::copy_to(TYPE *dest, int from, int to)
{
	// calculate the length to copy
	int len = to - from;
	if (len <= 0)
		return 0;

	if (to > count || from > count)
		return 0;

	int items_copied{0};
	// cannot copy more than LENGTH items:
	if (len > LENGTH)
		return 0;

	// detect when copy needs to be done twice - when data loops
	if (len + (from % LENGTH) > LENGTH)
	{
		// split into two memcpy operations
		// define section beginnings
		TYPE *sec_0 = buffer + (from % LENGTH);
		TYPE *sec_1 = buffer;

		// define section lengths
		int sec_0_len = LENGTH - from % LENGTH;
		int sec_1_len = len - sec_0_len;

		// copy data
		memcpy(dest, sec_0, sec_0_len * sizeof(TYPE));
		memcpy(dest + sec_0_len, sec_1, sec_1_len * sizeof(TYPE));

		items_copied += sec_0_len;
		items_copied += sec_1_len;
	}
	else
	{
		// can be read into contiguous memory
		memcpy(dest, buffer + from % LENGTH, len * sizeof(TYPE));
		items_copied += len;
	}
	return items_copied;
}

/**
 * samples_recv: Get total number of samples received.
 * @returns Total number of samples received.
 */
template <class TYPE, int LENGTH>
int Looping_Buffer<TYPE, LENGTH>::samples_recv()
{
	return count;
}