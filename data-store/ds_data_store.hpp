#pragma once

#include "ds_types.hpp"
#include "ds_looping_buffer.hpp"
/**
 * Reader
 * Identifies a data reader
 */
struct Reader
{
};

/**
 * Data_Store
 * Hold all recorded biometric data.
 * Coordinate data access to other objects in other threads.
 */
template <typename LENGTH>
class Data_Store
{
private:
public:
};