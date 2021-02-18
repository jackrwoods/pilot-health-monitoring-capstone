Make:
	g++ test.cpp -lbluetooth -g -o main.out
	./main.out

client:
	g++ test_client.cpp -lbluetooth -pthread -g -o test_client.out
	./test_client.out DC:A6:32:33:BD:F1

server:
	g++ test_server.cpp -lbluetooth -pthread -g -o test_server.out
	./test_server.out