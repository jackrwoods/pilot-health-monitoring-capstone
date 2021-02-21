#ifndef DATASOURCE_HPP
#define DATASOURCE_HPP
/* This file defines a "datasource" interface. This allows data consumers to be
 * abstracted from the details of the implementation of each source of data.
 * During testing, we might be using I2C and then decide to change the data
 * source later.
 */

#include <chrono>
#include <forward_list>
#include <functional>
#include <queue>

// An enumeration of all data sources. Currently, only the MAX30100 is implemented.
enum Source {
	MAX30100
	// Possibly add the ECE group's block as a "device" here?
};

// An enumeration containing various units of measure
enum UnitOfMeasure {
	CELSIUS,
	MS, // Milliseconds (ms)
	NONE // No units (No units are specified in the MAX30100 spec sheet)
};

struct Value {
	UnitOfMeasure unit;
	double value;
};

enum PilotState {
	UNSTRESSED,
	STRESSED
};

struct Sample {
	std::chrono::_V2::system_clock::time_point timestamp; // Timestamp corresponding to when this was created
	Source sourceType; // Describes what created this sample. ie: data interpreter, sensor, etc
	Value irLED; // Infrared LED observation
	Value redLED; // Red LED observation
	Value temperature; // Temperature observation
	Value bpm; // Heart rate
	Value spo2; // Blood oxygen content
	Value pilotState; // The enum for Pilot State can be cast to a double
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
			std::function<void(struct Sample*)> callbackFunction) {
			this->callbacks.push_front(callbackFunction);
		}
	protected:
		std::forward_list<std::function<void(struct Sample*)>> callbacks;
		std::queue<struct Sample*> unprocessedData;
};
#endif
