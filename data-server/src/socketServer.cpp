/* This file implements the websocket server which provides low-latency, real-
 * time PO2 data to the website frontend.
 */

#include "datasource.hpp"
#include <future>
#include "server_ws.hpp"
#include "max30100Datasource.cpp"

#include <ctime>
#include <cstdlib>

using WsServer = SimpleWeb::SocketServer<SimpleWeb::WS>;

WsServer server;

// Simple datasource callback.
// This produces a json string and sends it to all websocket clients.
void sendDataToAllClients(struct Sample *data)
{
	srand(time(NULL));

	std::string json = "{\"timestamp\": ";
	json += std::to_string(data->timestamp);
	json += ",\"temperature\": ";
	json += std::to_string(0);
	json += ", \"HR\": ";
	json += std::to_string(data->bpm);
	json += ", \"SpO2\": ";
	json += std::to_string(data->spo2);
	json += ", \"sentTimestamp\": ";
	json += std::to_string((unsigned long)std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now()).time_since_epoch().count());
	json += "}";

	// Send the latest datapoint to all clients
	for (auto &c : server.get_connections())
	{
		if (c->path == "/data")
		{
			c->send(json);
		}
	}
}

void startServer(Datasource *datasource)
{
	// Start the websocket server on port 8080 using 1 thread
	server.config.port = 8080;

	// Configure the ping endpoint
	auto &ping = server.endpoint["^/ping/?$"];
	ping.on_handshake = [](std::shared_ptr<WsServer::Connection> /*connection*/, SimpleWeb::CaseInsensitiveMultimap & /*response_header*/) {
		return SimpleWeb::StatusCode::information_switching_protocols;
		// Upgrade to websocket
	};
	ping.on_open = [](std::shared_ptr<WsServer::Connection> connection) {
		std::cout << "Server: Opened connection " << connection.get() << "\n";
	};
	// See RFC 6455 7.4.1. for status codes
	ping.on_close = [](std::shared_ptr<WsServer::Connection> connection, int status, const std::string & /*reason*/) {
		std::cout << "Server: Closed connection " << connection.get() << " with status code " << status << "\n";
	};
	ping.on_error = [](std::shared_ptr<WsServer::Connection> connection, const SimpleWeb::error_code &ec) {
		std::cout << "Server: Error in connection " << connection.get() << ". "
				  << "Error: " << ec << ", error message: " << ec.message() << "\n";
	};
	ping.on_message = [](std::shared_ptr<WsServer::Connection> connection, std::shared_ptr<WsServer::InMessage> msg) {
		connection->send(":)");
	};

	// Configure data endpoint
	auto &data = server.endpoint["^/data/?$"];
	data.on_open = [](std::shared_ptr<WsServer::Connection> connection) {
		std::cout << "Server: Opened connection " << connection.get() << "\n";
	};
	// See RFC 6455 7.4.1. for status codes
	data.on_close = [](std::shared_ptr<WsServer::Connection> connection, int status, const std::string & /*reason*/) {
		std::cout << "Server: Closed connection " << connection.get() << " with status code " << status << "\n";
	};
	// Can modify handshake response headers here if needed
	data.on_handshake = [](std::shared_ptr<WsServer::Connection> /*connection*/, SimpleWeb::CaseInsensitiveMultimap & /*response_header*/) {
		return SimpleWeb::StatusCode::information_switching_protocols;
		// Upgrade to websocket
	};
	// See http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/reference.html, Error Codes for error code meanings
	data.on_error = [](std::shared_ptr<WsServer::Connection> connection, const SimpleWeb::error_code &ec) {
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
	std::function<void(struct Sample *)> callback = sendDataToAllClients;
	datasource->registerCallback(callback);

	server_thread.join();
}
