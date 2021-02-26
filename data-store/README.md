## Using the Data Store

The data store allows for thread safe access to all sampled data received by the system. To create a Data_Store object:
```cpp
Data_Store<SAMPLE_TYPE, BUFFER_LENGTH> ds;
```

## Adding data

SAMPLE_TYPE defines what type of data the data store holds. BUFFER_LENGTH defines how many samples of SAMPLE_TYPE it will hold. The data store will always hold the BUFFER_LENGTH most recent SAMPLE_TYPE samples that it receives. To write these samples to the data buffer, first register the thread as the writer thread:

```cpp
void register_writer_thread();
```

This restricts writing data to the data store to only that specific thread. Attempts to write to the data store from an unregistered thread will fail silently. New data can be written to the data buffer in two ways:

```cpp
int new_data(SAMPLE_TYPE *src, size_t len);
int new_data(SAMPLE_TYPE s);
```

The data store also offers functionality for setting internal measured variables:
```cpp
void set_bpm_variance(uint32_t);
void set_bpm_average(uint32_t);
void set_po2_average(uint32_t);
void set_ece_bpm(uint32_t);
void set_ece_po2(uint32_t);
```
These methods are not restricted to any thread and can be used freely.


## Reading Data

Reader threads must also be registered:

```cpp
void register_reader_thread()
```

This allocates data buffers and tracks the last read sample of each thread. To get the number of available unread samples for a thread:

```cpp
int available_samples();
```

Data buffers for threads are stored in standard library vectors. Access to these vector buffers can gained be through iterator or reference:

```cpp
// reference
std::vector<SAMPLE_TYPE> &data = ds.vec();

// iterator
for (auto vi = ds.begin(); vi != ds.end(); vi++)
{
    SAMPLE_TYPE sample = *vi;
}
```

Be careful using these, data copies to the vector buffers while references or iterators are being used may result in errors. These data copies occur automatically and only at the creation of a begin() vector iterator or at a vec() call. Specifically, don't do this:

```cpp
for (auto vi = ds.begin(); vi != ds.end(); vi++)
{
    std::vector<SAMPLE_TYPE> = ds.vec(); // WRONG
    SAMPLE_TYPE sample = *vi;
}
```

Any available data will be copied into the vector buffer at the ds.vec() call. Any old data will be lost and the vector iterator may be invalidated, leading to errors. To bypass thread tracking and grab samples beginning with the oldest available sample:

```cpp
int copy(SAMPLE_TYPE *s, size_t len)
```

len samples will be copied to the SAMPLE_TYPE pointer s. Calculated values can be retreived by any thread using the methods:

```cpp
uint32_t get_bpm_variance() const;
uint32_t get_bpm_average() const;
uint32_t get_po2_average() const;
uint32_t get_ece_bpm() const;
uint32_t get_ece_po2() const;
```

## Issues

There is a possibility of a race condition during registering of threads. If thread A currently holds an iterator or reference to its data buffer while thread B registers itself as a reader AND triggers a hash table expansion, thread A's iterator or reference will become invalid. To combat this a std::mutex is used while registering threads (to prevent collisions during registration) and 16 buckets are pre-allocated for the hash table. This is more of a work around I just can't think of a better way to do it right now. If more than 16 threads are being used the number of buckets should be adjusted accordingly.