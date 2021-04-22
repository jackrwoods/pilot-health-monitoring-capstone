## Summary

Simulate output of a Max30100 pulse oximeter.

## Compilation

Requires the [BlueZ](http://www.bluez.org/) stack and [WiringPi](http://wiringpi.com/). Compile with `-lbluetooth`, `-lwiringPi`, and `-lpthread`.

Compile with the given Makefile:  
`make client`: Create a client test executable `test_client.out`  
`make server`: Create a server test executable `test_server.out`  
`make sensor`: Create a mock PO2 sensor executable `mock_po2_sensor.out`  
`make sensor_server`: Create a server executable for receiving mock PO2 data `mock_pow_sensor_server.out`  
`make list`: Create a Bluetooth device listing executable `list.out`  

## Usage

Client and Server executables should be run on separate devices. Server executables do not require command line arguments. Client executables require a single Bluetooth address to connect to:

```bash
./mock_po2_sensor.out XX:XX:XX:XX:XX:XX
./test_client.out XX:XX:XX:XX:XX:XX
```

```bash
./mock_po2_sensor_server.out
./test_server.out
```