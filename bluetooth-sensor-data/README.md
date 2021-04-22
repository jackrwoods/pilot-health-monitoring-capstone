Bluetooth packet format for storing samples

#define BYTE_IDENTIFIER_timestamp   0x01
#define BYTE_IDENTIFIER_sourceType  0x02
#define BYTE_IDENTIFIER_irLED       0x03
#define BYTE_IDENTIFIER_redLED      0x04
#define BYTE_IDENTIFIER_temperature 0x05
#define BYTE_IDENTIFIER_bpm         0x06
#define BYTE_IDENTIFIER_avg_bpm     0x07
#define BYTE_IDENTIFIER_spo2        0x08
#define BYTE_IDENTIFIER_pilotState  0x09


Each measurement in the packet begins with its byte identifier. Each piece of data is stored as a double - 8 bytes.
a timestamp identifier defines the beginning of a new sample within the packet.

any other identifier means the next 8 bytes are the measurement for that metric in the current packet
other metrics are optional. A packet can be received that contains only a timestamp.

Packets constructed from the functions included in sample_types.hpp will include timestamp and sourceType by default