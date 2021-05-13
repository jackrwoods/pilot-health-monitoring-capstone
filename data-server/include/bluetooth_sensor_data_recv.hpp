#pragma once

#include <iostream>
#include <thread>

#include "../include/bluetooth/bluetooth_con.hpp"
#include "datasource.hpp"
#include "../include/bluetooth_utils.hpp"

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

	while (!quit_receive_thread)
	{
		if (c.available())
		{
			// grab all available bluetooth packets
			std::vector<PHMS_Bluetooth::Packet> v = c.get_all();
			received_samples += v.size();

			long time = (long)std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now()).time_since_epoch().count();

			// for each bluetooth packet received, get the samples
			for (auto i : v)
			{
				auto smp = sample_buffer_from_bt_packet(i);
				std::vector<Sample> samples = smp.samples;
				int source = smp.src;

				// TODO: mitigate sensor malfunction here

				// for each sample, call all of the callback functions
				for (auto s : samples)
				{
					s.timestamp = time;
					// Pass a pointer to the latest data to all of the callback functions.
					for (auto clb : callbacks)
						clb(&s);
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