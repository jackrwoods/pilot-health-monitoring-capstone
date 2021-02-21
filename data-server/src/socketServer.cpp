/* This file implements the websocket server which provides low-latency, real-
 * time PO2 data to the website frontend.
 */

#include "datasource.hpp"
#include <future>
#include "server_ws.hpp"
#include "max30100Datasource.cpp"

using WsServer = SimpleWeb::SocketServer<SimpleWeb::WS>;

class WebSocketServer {
	public:

		WsServer server;

		// Simple datasource callback.
		// This produces a json string and sends it to all websocket clients.
		void sendDataToAllClients(struct Sample* data) {
			if (data->deviceType == MAX30100) {
				std::string json = "{\"timestamp\": ";
				json += std::to_string(data->timestamp.time_since_epoch().count());
				json += ",\"temperature\": ";
				json += std::to_string(data->temperature.value);
				json += ", \"ir\": ";
				json += std::to_string(data->irLED.value);
				json +=	", \"r\": ";
				json += std::to_string(data->redLED.value);
				json += ", \"sentTimestamp\": ";
				json += std::to_string(std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now()).time_since_epoch().count());
				json += "}";

				// Send the latest datapoint to all clients
				for (auto &c : this->server.get_connections()) {
					c->send(json);
				}
			}
		}

		WebSocketServer(Datasource* ds) {
			// Start the websocket server on port 8080 using 1 thread
			this->server.config.port = 8080;

			auto &echo = this->server.endpoint["^/echo/?$"];
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
				this->server.start([&server_port](unsigned short port) {
					server_port.set_value(port);
				});
			});

			std::cout << "Server listening on " << server_port.get_future().get()
				<< "\n";

			// Listen to the datasource
			std::function<void(struct Sample*)> callback = sendDataToAllClients;
			ds->registerCallback(callback);
		}

};

