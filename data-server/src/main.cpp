/*
 * This instantiates a datasource, data store, and webserver and essentially
 * connects everything together.
 */

#include "datasource.hpp"
#include "max30100Datasource.cpp"
#include "socketServer.cpp"
#include "ds_data_store.hpp"

int main() {
	std::cout << "Starting up...\n";
	Max30100 datasource;

	std::cout << "Registering WebSocket callback...\n";
	WebSocketServer* ws = new WebSocketServer(&datasource);

	std::cout << "Registering data store callback...\n";
	Data_Store* ds = new Data_Store(&datasource);

	std::cout << "Reading from datasource. \n";
	datasource.initializeConnection();

	while(1); // Run forever!

	return 0;
}
