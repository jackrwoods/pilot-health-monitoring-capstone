#pragma once

#include <iostream>
#include <thread>

#include "../include/bluetooth/bluetooth_con.hpp"
#include "datasource.hpp"
#include "../include/bluetooth_utils.hpp"

#define MITIGATE_SENSOR_MALFUNCTION 1

class BluetoothReceiver : public Datasource
{
private:
	bool quit_receive_thread{false};
	int received_samples{0};

	PHMS_Bluetooth::Communicator c;
	std::string bluetooth_address;

	std::thread callback_thread;

	bool connection_initialized{false};

	void run_receive();

public:
	BluetoothReceiver();
	~BluetoothReceiver();

	void initializeConnection();

	void send_pilot_state(uint8_t state);

	void set_bt_address(const std::string &s);
};

void BluetoothReceiver::run_receive()
{
	// crash if no connection was made
	if (!connection_initialized)
	{
		std::cerr << "(BluetoothReceiver) Connection uninitialized at call to run_receive" << std::endl;
		exit(1);
	}

	int samples_before_analysis{100};
	int errors[16]{0};
	bool sensor_seen[16]{false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};
	bool sensor_valid[16] = {true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true};
	int error_threshold{500};
	int active_sensor{0};

	while (!quit_receive_thread)
	{
		if (c.available())
		{
			// grab all available bluetooth packets
			std::vector<PHMS_Bluetooth::Packet> v = c.get_all();
			received_samples += v.size();

			long time = (long)std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now()).time_since_epoch().count();

			// for each bluetooth packet received, get the samples
			// keep track of errors at each sensor

			for (auto i : v)
			{
				auto smp = sample_buffer_from_bt_packet(i);
				std::vector<Sample> samples = smp.samples;
				int source = smp.src;

				if (!sensor_seen[source])
				{
					std::cout << "New connected sensor: " << source << std::endl;
					sensor_seen[source] = true;
				}

// quickly disable error checking
#ifdef MITIGATE_SENSOR_MALFUNCTION
				// mitigate sensor malfunction

				for (auto s : samples)
				{
					bool er{false};
					if (samples_before_analysis == 0)
					{
						// do error checking calculions - if a sample is outside of normal range, increment error count and move on
						// ranges are based on calculations on recorded data
						if (s.spo2 > 14200 || s.spo2 < 13700)
						{
							// std::cout << "spo2 error was found in sensor " << source << " total errors from this sensor: " << errors[source] << std::endl;
							errors[source] += 1;
							er = true;
						}
						if (s.irLED > 14400 || s.irLED < 13660)
						{
							// std::cout << "irled error was found in sensor " << source << " total errors from this sensor: " << errors[source] << std::endl;
							errors[source] += 1;
							er = true;
						}
						if (s.redLED > 14400 || s.redLED < 13660)
						{
							// std::cout << "redled error was found in sensor " << source << " total errors from this sensor: " << errors[source] << std::endl;
							errors[source] += 1;
							er = true;
						}
					}
					else
					{
						// std::cout << "sba: " << samples_before_analysis << std::endl;
						samples_before_analysis--;
					}

					// slowly roll back erroring sensors
					if (errors[source] >= 10 && !er)
						errors[source] -= 10;

					// std::cout << "errors at " << source << " " << errors[source] << std::endl;

					// if a sensor is past the error threshold do not consider its samples

					// reassign active sensor if errors are detected
					if (!sensor_valid[active_sensor])
					{
						bool found{false};
						for (int i = 0; i < 16; i++)
						{
							if (sensor_seen[i] && sensor_valid[i])
							{
								found = true;
								active_sensor = i;
								break;
							}
						}

						if (!found)
						{
							std::cerr << "No valid sensors attached (fatal)\n";
							exit(1);
						}
						else
						{
							std::cout << "responding to invalid sensor: new primary sensor is " << active_sensor << std::endl;
						}
					}

					if (errors[source] > error_threshold && sensor_valid[source])
					{
						std::cout << "sensor " << source << " errored out\n";
						sensor_valid[source] = false;
						continue;
					}

					if (source != active_sensor)
						continue;

#endif

					// for each sample, call all of the callback functions
					uint16_t last_spo2{95};
					for (auto s : samples)
					{
						// calculate spo2 for each sample based on irled and redled - https://github.com/oxullo/Arduino-MAX30100/blob/master/src/MAX30100_SpO2Calculator.cpp
						const uint8_t spO2LUT[43] = {100, 100, 100, 100, 99, 99, 99, 99, 99, 99, 98, 98, 98, 98,
													 98, 97, 97, 97, 97, 97, 97, 96, 96, 96, 96, 96, 96, 95, 95,
													 95, 95, 95, 95, 94, 94, 94, 94, 94, 93, 93, 93, 93, 93};

						float acSqRatio = 100.0 * log(s.redLED / received_samples) / log(s.irLED / received_samples);
						uint8_t index = 0;

						if (acSqRatio > 66)
						{
							index = (uint8_t)acSqRatio - 66;
						}
						else if (acSqRatio > 50)
						{
							index = (uint8_t)acSqRatio - 50;
						}

						if (index > 42 || index < 0)
							s.spo2 = last_spo2;
						else
							s.spo2 = spO2LUT[index];
						last_spo2 = s.spo2;

						// TODO: DETERMINE PILOT STATE here =================================================
						if (s.bpm > 70)
							s.pilot_state = 1;

						// ==================================================================================


						s.timestamp = time;
						// Pass a pointer to the latest data to all of the callback functions.
						for (auto clb : callbacks)
							clb(&s);
					}
				}
			}
		}
	}
}

BluetoothReceiver::BluetoothReceiver()
{
	std::cout << "Bluetooth Sample Reciever ('hcitool dev' in terminal returns the bluetooth address of this device)" << std::endl;
}

BluetoothReceiver::~BluetoothReceiver()
{
	std::cout << "Killing bluetooth thread...\n";
	quit_receive_thread = true;
	c.quit();
	callback_thread.join();
}

void BluetoothReceiver::initializeConnection()
{
	if (bluetooth_address.empty())
	{
		std::cerr << "(BluetoothReceiver) call to InitializeConnection with no bluetooth address set" << std::endl;
		exit(1);
	}

	// run a thread that sits and receives packets until the application quits
	if (c.open_con(bluetooth_address, 5) == 0)
	{
		connection_initialized = true;
		c.run();
	}
	else
	{
		std::cerr << "(BluetoothReceiver) an error occurred opening connection to " << bluetooth_address << std::endl;
		exit(1);
	}

	callback_thread = std::thread(&BluetoothReceiver::run_receive, this);
}

/**
 * send_pilot_state: Send a stressed or unstressed pilot state to the connected collection device
 * @param state: 1 if the pilot is stressed, 0 if the pilot is unstressed
 */
void BluetoothReceiver::send_pilot_state(uint8_t state)
{
	printf("sending pilot state: %s\n", (state ? "stressed" : "unstressed"));
	c.push(&state, 1);
}

void BluetoothReceiver::set_bt_address(const std::string &s)
{
	bluetooth_address = s;
}