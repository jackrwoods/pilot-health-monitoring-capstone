#ifndef BLUETOOTHDATASOURCE
#define BLUETOOTHDATASOURCE

#include <iostream>
#include <thread>

#include "../include/bluetooth/bluetooth_con.hpp"
#include "datasource.hpp"
#include "../include/bluetooth_utils.hpp"

class BluetoothReceiver : public Datasource {
private:
	bool quit_receive_thread{ false };
	int received_samples{ 0 };
	std::thread* collector;

	void run_receive()
	{
		PHMS_Bluetooth::Server s;
		s.open_con();
		std::thread bt_thread(&PHMS_Bluetooth::Server::run, &s);
		
		// Empty callback pointer
		std::forward_list<std::function<void(struct Sample*)>>::iterator callback;

		while (!quit_receive_thread) {

			if (s.available()) {
				// grab all available bluetooth packets
				std::vector<PHMS_Bluetooth::Packet> v = s.get_all();
				received_samples += v.size();

				// for each bluetooth packet received, get the samples
				for (auto i : v) {
					std::vector<Sample> samples = sample_buffer_from_bt_packet(i);

					// for each sample, call all of the callback functions
					for (auto s : samples)
						// Pass a pointer to the latest data to all of the callback functions.
						for (callback = this->callbacks.begin(); callback != this->callbacks.end(); callback++) {
							(*callback)(data);
					}
				}
			}
		}
		s.quit();
		bt_thread.join();
	}
public:
	BluetoothReceiver() {
		std::cout << "Mock PO2 Sensor Recieve test ('hcitool dev' in terminal returns the bluetooth address of this device)" << std::endl;

		// run a thread that sits and receives packets until the application quits
		this->collector = new std::thread(&BluetoothReceiver::run_receive,this);
		this->collector->detach();
	}

	~BluetoothReceiver() {
		std::cout << "Killing bluetooth thread...\n";
		this->quit_receive_thread = true;
	};
};

#endif