#ifndef MAX30100DATASOURCE
#define MAX30100DATASOURCE

/* This file implements the "datasource" interface/abstract class. This
 * collects and processes raw sample data from the MAX 30100 sensor over an
 * I2C connection.
 */

// Import libraries
#include <chrono>
#include <functional>
#include <iostream>
#include <thread>
#include <wiringPiI2C.h>

// Import datasource class definition
#include "datasource.hpp"

class Max30100: public Datasource {
	private:
		int fd;
		static const int DEVICE_ID = 1;
		static const int DEVICE_ADDRESS = 0x57;
		static const int MODE_CONFIG_REG = 0x06;
		static const int SPO2_CONFIG_REG = 0x07;
		static const int LED_CONFIG_REG = 0x09;
		static const int TEMP_REG = 0x16;
		static const int TEMP_FRAC_REG = 0x17;
		static const int FIFO_BUF_PTR = 0x05;

		// Threaded processing
		bool running = false;
		std::thread* collector;
		std::thread* processor;


		void dataCollectionRunner() {

			// Keep track of time
			std::chrono::high_resolution_clock::time_point t;

			// Forever loop
			while(this->running) {
				// Only want to read at most 100hz
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
				t =	std::chrono::high_resolution_clock::now();

				// Create new sample struct
				struct Sample* data = new struct Sample();
				data->timestamp = (long) std::chrono::time_point_cast<std::chrono::milliseconds>(t);
				data->sourceType = MAX30100;
				// Read temp data
				data->temperature.unit = CELSIUS;
				data->temperature.value = (double)
					(wiringPiI2CReadReg8(this->fd, this->TEMP_REG) +
					((double) wiringPiI2CReadReg8(this->fd,
											this->TEMP_FRAC_REG) / 16));

				// Read IR and RED Data
				int ir = wiringPiI2CReadReg8(this->fd, this->FIFO_BUF_PTR) << 8;
				ir += wiringPiI2CReadReg8(this->fd,	this->FIFO_BUF_PTR);
				data->irLED.value = (double) ir;
				data->irLED.unit = NONE;

				int r = wiringPiI2CReadReg8(this->fd, this->FIFO_BUF_PTR) << 8;
				r += wiringPiI2CReadReg8(this->fd, this->FIFO_BUF_PTR);
				data->redLED.value = (double) r;
				data->redLED.unit = NONE;

				// Add to the queue
				this->unprocessedData.push(data);

				// Reset the temperature reading
				wiringPiI2CWriteReg8(this->fd,
					this->MODE_CONFIG_REG, 0b00001011); //00001011b = 11
			}

		};

		void dataProcessorRunner() {
			// Forever loop
			while (this->running) {

				if (this->unprocessedData.size() > 0) {

					struct Sample* data = this->unprocessedData.front();
					this->unprocessedData.pop();

					// Pass a pointer to the latest data to all of the
					// callback functions.
					// When they are all done executing, delete the data.
					std::forward_list<std::function<void(struct Sample*)>>::iterator callback;
					for (callback = this->callbacks.begin(); callback != this->callbacks.end(); callback++) {
						(*callback)(data);
					}

					delete data;
				}

			}
		};

	public:
		Max30100() {

			// Initialize WiringPi
			this->fd = wiringPiI2CSetup(this->DEVICE_ADDRESS);
			if (this->fd < 0) throw this->fd;

			// Reset the sensor
			wiringPiI2CWriteReg8(this->fd, this->MODE_CONFIG_REG,
				0b01000000); // 01000000b = 64

			// Set the sensor mode to SPO2 and enable temperature
			wiringPiI2CWriteReg8(this->fd, this->MODE_CONFIG_REG,
				0b00001011); //00001011b = 11

			// Sets:
			//  - High Resolution (16 bit) mode
			//  - 100hz Sample Rate
			//  - 1.6ms LED Pulse Width (req'd for high res)
			wiringPiI2CWriteReg8(this->fd, this->SPO2_CONFIG_REG,
				0b01000111); // 01000111b = 71

			// Set the LED to the highest current level (50 mA)
			wiringPiI2CWriteReg8(this->fd, this->LED_CONFIG_REG,
				0b11111111);


		};

		~Max30100() {
			std::cout << "Killing threads...\n";
			this->running = false;
		};

		void initializeConnection() {

			// Keep the threads running
			this->running = true;

			// Spawn a thread that reads the data
			this->collector = new std::thread(&Max30100::dataCollectionRunner,
				this);
			this->collector->detach();

			// Spawn a thread that processes the data
			this->processor = new std::thread(&Max30100::dataProcessorRunner,
				this);
			this->processor->detach();
		};

};

#endif
