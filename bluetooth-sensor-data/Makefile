Make:
	echo #
	# send - Compile program for reading from a csv file and sending over bluetooth connection
	#        Requires command line arguments: ./bluetooth_sensor_data_send.out [bluetooth address] [input file]
	#        input file must be in csv format conforming to this format:
	#        timestamp, bpm, average bpm, red, ir, HR, HRvalid, SPO2, SPO2Valid
	#
	# recv - Compile program for receiving sent data
	#        sent data must conform to this format:
	#        undecided

send:
	g++ bluetooth_sensor_data_send.cpp -lpthread -lbluetooth -lncurses -g -o bluetooth_sensor_data_send.out

recv:
	g++ bluetooth_sensor_data_recv.cpp -lpthread -lbluetooth -g -o bluetooth_sensor_data_recv.out
