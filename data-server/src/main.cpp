/*
 * This instantiates a datasource, data store, and webserver and essentially
 * connects everything together.
 */

#include "datasource.hpp"
#include "max30100Datasource.cpp"
#include "socketServer.cpp"
#include "ds_data_store.hpp"
#include "sql_con.hpp"

int main() {
	std::cout << "Starting up...\n";
	Max30100 datasource;

	std::cout << "Registering data store callback...\n";
	Data_Store<Sample>* ds = new Data_Store<Sample>(&datasource);

	std::cout << "Registering WebSocket callback...\n";
	std::thread* server = new std::thread(&startServer, &datasource);

	std::cout << "Starting DB thread...";
	SQL_Connection* db = new SQL_Connection();
	std::function<void(struct Sample*)> dbCallback(std::bind(&SQL_Connection::insert_sample, db, std::placeholders::_1));
	datasource.registerCallback(dbCallback);

	std::cout << "Reading from datasource. \n";
	datasource.initializeConnection();

	while(1) {
		std::this_thread::sleep_for(std::chrono::milliseconds(10000));
		std::cout << "Selected " << db->select_all_samples() << " samples from DB in 10 seconds.\n";
	}

	return 0;
}
