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

	std::thread collector;

	PHMS_Bluetooth::Client c;
	PHMS_Bluetooth::Server s;

	void run_receive();

public:
	BluetoothReceiver()
	{
		std::cout << "Bluetooth Sample Recieve ('hcitool dev' in terminal returns the bluetooth address of this device)" << std::endl;
	}

	void initializeConnection()
	{
		// run a thread that sits and receives packets until the application quits
		collector = std::thread(&BluetoothReceiver::run_receive, this);
		collector.detach();
	}

	~BluetoothReceiver()
	{
		std::cout << "Killing bluetooth thread...\n";
		quit_receive_thread = true;
	};

	/**
	 * send_pilot_state: Send a stressed or unstressed pilot state to the connected collection device
	 * @param state: 1 if the pilot is stressed, 0 if the pilot is unstressed
	 */
	void send_pilot_state(uint8_t state)
	{
		c.push(&state, 1);
	}
};

void BluetoothReceiver::run_receive()
{
	// open server connection, connects to any device that requests connection
	s.open_con();
	std::thread bt_thread(&PHMS_Bluetooth::Server::run, &s);

	// start a client connection to the connected bluetooth device for sending pilot states
	c.open_con(s.get_connected_address());
	std::thread client_thread(&PHMS_Bluetooth::Client::run, &c);

	while (!quit_receive_thread)
	{

		if (s.available())
		{
			// grab all available bluetooth packets
			std::vector<PHMS_Bluetooth::Packet> v = s.get_all();
			received_samples += v.size();

			long time = (long)std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now()).time_since_epoch().count();

			// for each bluetooth packet received, get the samples
			for (auto i : v)
			{
				std::vector<Sample> samples = sample_buffer_from_bt_packet(i);

				// for each sample, call all of the callback functions
				for (auto s : samples)
				{
					s.timestamp = time;
					// Pass a pointer to the latest data to all of the callback functions.
					for(auto c : callbacks)
						c(&s);
				}
			}
		}
	}
	s.quit();
	bt_thread.join();

	c.quit();
	client_thread.join();
}