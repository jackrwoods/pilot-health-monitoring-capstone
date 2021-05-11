/*
 * This instantiates a datasource, data store, and webserver and essentially
 * connects everything together.
 */

#include "bluetooth_sensor_data_recv.hpp"
#include "datasource.hpp"
#include "max30100Datasource.cpp"
#include "socketServer.cpp"
#include "ds_data_store.hpp"
#include "sql_con.hpp"

int main()
{
	std::cout << "Starting up...\n";
	BluetoothReceiver datasource;

	// set the bluetooth address before initializing the connection
	// TODO: get this from command line argument?
	datasource.set_bt_address("B8:27:EB:49:35:BC");

	std::cout << "Registering data store callback...\n";
	Data_Store<Sample> *ds = new Data_Store<Sample>(&datasource);

	std::cout << "Registering WebSocket callback...\n";
	std::thread *server = new std::thread(&startServer, &datasource);

	std::cout << "Starting DB thread...";
	SQL_Connection *db = new SQL_Connection();

	std::cout << "Reading from datasource. \n";
	datasource.initializeConnection();

	// This job runs indefinitely.
	// It inserts samples into the sqlite database in batches.
	ds->register_reader_thread(); // How data_store tracks which samples have not been read yet

	// fake pilot state to send
	int state {0};
	while (true)
	{
		// Flush buffered samples to db twice per second
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		auto vec = ds->vec();
		if(vec.size() > 0)
			db->insert_samples(vec);
		// printf("flushed to database\n");
		datasource.send_pilot_state(state++ % 2);
	}

	return 0;
}
