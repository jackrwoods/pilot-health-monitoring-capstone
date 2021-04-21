## Summary

Super basic bluetooth client / server code. Uses L2CAP sockets as described [here](https://people.csail.mit.edu/albert/bluez-intro/x559.html).

## Compilation

Requires the [BlueZ](http://www.bluez.org/) stack. Compile with `-lbluetooth`.

## Usage

To create a Bluetooth client:

```cpp
PHMS_Bluetooth::Client client;
client.open_con(std::string bluetooth_address);
client.run();
```

To send data using the Bluetooth client:

```cpp
client.push(Packet(size_t length, void *src));
```

or:

```cpp
client.push(Packet(const Packet &));
```

To stop the bluetooth client:

```cpp
client.quit()
```

To create a Bluetooth server:

```cpp
PHMS_Bluetooth::Server server;
server.open_con();
server.run();
```

The server will connect to any device that attempts a connection. From there it will receive and store received Bluetooth packets. To retrieve received Bluetooth packets:

```cpp
std::vector<Packet> = s.get_all();
```

To stop the bluetooth server:
```cpp
server.quit()
```

In general, it is best to run each Client or Server instance in its own thread. Usage examples in `test_client.cpp` and `test_server.cpp`.