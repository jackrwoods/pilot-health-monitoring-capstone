Make:
	echo # 
	# Specify target (client, server, sensor).
	# client: Test - sends string input to a Bluetooth address specified by command line argument.
	# server: Test - acepts incoming Bluetooth connections. Accepts any connection. Prints all data to screen after connection closes.
	# sensor: Generates mock blood oxygen data and sends to Bluetooth address specified by command line argument.
	# sensor_client: Accepts mock blood oxygen data and saves to csv file.
	# list: Print a list of available Bluetooth addresses.

client:
	g++ ./bluetooth/test_client.cpp -lbluetooth -pthread -g -o test_client.out
	# client test compiled to ./test_client.out

server:
	g++ ./bluetooth/test_server.cpp -lbluetooth -pthread -g -o test_server.out
	# server test compiled to ./test_server.out

sensor:
	g++ mock_po2_sensor.cpp -lbluetooth -pthread -lwiringPi -g -o mock_po2_sensor.out
	# sensor send test compiled to ./mock_po2_sensor.out

sensor_server:
	g++ mock_po2_sensor_server.cpp -lbluetooth -pthread -g -o mock_po2_sensor_server.out
	# sensor receive test compiled to mock_po2_sensor_server.out

list:
	g++ list.cpp -lbluetooth -g -o list.out
	# list program compiled to list.out