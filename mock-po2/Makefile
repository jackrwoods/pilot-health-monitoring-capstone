Make:
	# Specify target (client, server, sensor).
	# client: Test - sends string input to a bluetooth address specified by command line argument.
	# server: Test - acepts incoming bluetooth connections. Accepts any connection. Prints all data to screen after connection closes.
	# sensor: Generates mock blood oxygen data and sends to bluetooth address specified by command line argument.
	# sensor_client: Accepts mock blood oxygen data and saves to csv file.

client:
	g++ ./bluetooth/test_client.cpp -lbluetooth -pthread -g -o test_client.out
	# client test compiled to ./test_client.out

server:
	g++ ./bluetooth/test_server.cpp -lbluetooth -pthread -g -o test_server.out
	# server test compiled to ./test_server.out

sensor:
	g++ mock_po2_sensor.cpp -lbluetooth -pthread -g -o mock_po2_sensor.out
	# sensor send test compiled to ./mock_po2_sensor.out

sensor_client:
	g++ mock_po2_sensor_client.cpp -lbluetooth -pthread -g -o mock_po2_sensor_client.out
	# sensor receive test compiled to mock_po2_sensor_client.out