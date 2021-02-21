#
This directory contains files for collecting data from the Max30100 PO2 sensor, and provides some interfaces which can be used to create new datasources and data consumers.

## Compiling the Code
You can use `make socketServer` to build the websocket server and `make simpleDataCollector` to create a simple application that just prints PO2 data to the console in CSV format.

## Dependencies

	- WiringPi should come pre-installed on Raspbian systems.
	- Latest libssl
	- Boost - `sudo apt-get install libboost1.62-*`
	- `sqlite3` and `libsqlite3-dev`
