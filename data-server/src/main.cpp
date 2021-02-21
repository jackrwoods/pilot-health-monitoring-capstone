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

	std::cout << "Reading from datasource. \n";
	datasource.initializeConnection();

	std::cout << "Registering data store callback...\n";
	Data_Store<Sample>* ds = new Data_Store<Sample>(&datasource);

	std::cout << "Registering WebSocket callback...\n";
	startServer(&datasource);

	return 0;
}
