/* This file defines a small application which:
 * 	- Opens an I2C connection and starts collecting data
 * 	- Connects to a socket server on the provided ip:port
 * 	- Sends the data to the server over the socket server
 */

#include "max30100Datasource.cpp"
#include "datasource.hpp"

void printData(struct RawOutput* data) {
	std::cout << "Data: " << data->timestamp << ", " << data->values[0].value
		<< ", " << data->values[1].value << ", " << data->values[2].value
		<< "\n";
}

int main() {

	Max30100 datasource;

	std::function<void(struct RawOutput*)> callback = printData;

	datasource.registerCallback(callback);

	datasource.initializeConnection();

	while(1) {

	}

	return 0;
}
