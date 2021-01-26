/* This file implements the websocket server which provides low-latency, real-
 * time PO2 data to the website frontend.
 */

#include "datasource.hpp"
#include <future>
#include "server_ws.hpp"
#include "max30100Datasource.cpp"

using WsServer = SimpleWeb::SocketServer<SimpleWeb::WS>;

WsServer server;

// Simple datasource callback.
// This produces a json string and sends it to all websocket clients.
void sendDataToAllClients(struct RawOutput* data) {
	if (data->deviceType == MAX30100) {
		std::string json = "{\"timestamp\": ";
		json += std::to_string(data->timestamp);
		json += ",\"temperature\": ";
		json += std::to_string(data->values[0].value);
		json += ", \"ir\": ";
		json += std::to_string(data->values[1].value);
		json +=	", \"r\": ";
		json += std::to_string(data->values[2].value);
		json += ", \"sentTimestamp\": ";
		json += std::to_string(std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now()).time_since_epoch().count());
		json += "}";

		// Send the latest datapoint to all clients
		for (auto &c : server.get_connections()) {
			c->send(json);
		}
	}
}

int main() {
	// Start the websocket server on port 8080 using 1 thread
	server.config.port = 8080;

	auto &echo = server.endpoint["^/echo/?$"];
	echo.on_open = [](std::shared_ptr<WsServer::Connection> connection) {
		std::cout << "Server: Opened connection " << connection.get() << "\n";
	};
	// See RFC 6455 7.4.1. for status codes
	echo.on_close = [](std::shared_ptr<WsServer::Connection> connection, int status, const std::string & /*reason*/) {
		std::cout << "Server: Closed connection " << connection.get() << " with status code " << status << "\n";
	};
	// Can modify handshake response headers here if needed
	echo.on_handshake = [](std::shared_ptr<WsServer::Connection> /*connection*/, SimpleWeb::CaseInsensitiveMultimap & /*response_header*/) {
		return SimpleWeb::StatusCode::information_switching_protocols;
		// Upgrade to websocket
	};
	// See http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/reference.html, Error Codes for error code meanings
	echo.on_error = [](std::shared_ptr<WsServer::Connection> connection, const SimpleWeb::error_code &ec) {
		std::cout << "Server: Error in connection " << connection.get() << ". "
			<< "Error: " << ec << ", error message: " << ec.message() << "\n";
	};

	std::promise<unsigned short> server_port;
	std::thread server_thread([&server_port]() {
		// Start server
		server.start([&server_port](unsigned short port) {
			server_port.set_value(port);
		});
	});

	std::cout << "Server listening on " << server_port.get_future().get()
		<< "\n";



	// Set up the datasource
	Max30100 datasource;
	std::function<void(struct RawOutput*)> callback = sendDataToAllClients;
	datasource.registerCallback(callback);
	datasource.initializeConnection();

	server_thread.join();

	return 0;
}
