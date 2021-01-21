#ifndef DATASOURCE_HPP
#define DATASOURCE_HPP
/* This file defines a "datasource" interface. This allows data consumers to be
 * abstracted from the details of the implementation of each source of data.
 * During testing, we might be using I2C and then decide to change the data
 * source later.
 */

#include <forward_list>
#include <functional>
#include <queue>

// The type of device that is the source for this data.
enum Device {
	MAX30100,
	OTHER // TODO: JW - This is a placeholder for now.
};

// An enumeration containing various units of measure
enum UnitOfMeasure {
	CELSIUS,
	MS // Milliseconds (ms)
};

struct Sample {
	UnitOfMeasure unit;
	double value;
};

// Defines a generic datatype for PO2 data
struct RawOutput {
	long timestamp; // Linux epoch timestamp in milliseconds (ms)
	Device deviceType; // One of the devices listed in the enumeration
	Sample* values; // An array of raw values read from a device
	int length; // Length of the values array
};

class Datasource {
	public:
		// Spawns two threads:
		// 	1. A data collection thread that reads samples at a
		// 	set rate and pushes them to this->unprocessedSamples
		// 	2. A data processing thread that calls each void
		// 	pointer in this->callbacks once for each sample
		virtual void initializeConnection() = 0;

		// Registers a callback function with this datasource.
		// For each datum collected from the sensor, a pointer
		// to the sample will be passed to the callback function.
		void registerCallback(
			std::function<void(struct RawOutput*)> callbackFunction) {
			this->callbacks.push_front(callbackFunction);
		}
	protected:
		std::forward_list<std::function<void(struct RawOutput*)>> callbacks;
		std::queue<struct RawOutput*> unprocessedData;
};
#endif
