#include <iostream>
#include <thread>
#include <unistd.h>

#include "./bluetooth/bluetooth_con.hpp"
#include "./mock_po2.hpp"
#include "./button_input.hpp"

// samples per second to send
#define SAMPLE_RATE 64

// samples per packet
#define PACKET_SIZE 32

// how many seconds should data be sent for
#define SECONDS 10

// amount to change po2 by on button presse
#define PO2_CHANGE 10

// global - makes other stuff easier
Mock_PO2_Sensor mock_sensor;

void increase_po2()
{
    std::cout << "Increasing PO2 moving average by " << PO2_CHANGE << std::endl;
    mock_sensor.po2_adjust(PO2_CHANGE);
}

void decrease_po2()
{
    std::cout << "Decreasing PO2 moving average by -" << PO2_CHANGE << std::endl;
    mock_sensor.po2_adjust(-PO2_CHANGE);
}

int main(int argc, char *argv[])
{
    std::cout << "Mock PO2 Sensor Send test" << std::endl;

    if (argc != 2)
    {
        std::cerr << "Exactly one command line argument required: ./test_client.out [blueooth address]" << std::endl;
        return 1;
    }

    // open bluetooth connection
    PHMS_Bluetooth::Client c;
    int con_stat = c.open_con(std::string(argv[1]));
    if (con_stat == -1)
    {
        std::cerr << "Error opening connection to bluetooth device at " << argv[1] << '.' << std::endl;
        return 1;
    }

    // spawn bluetooth communication in a new thread
    std::thread client(&PHMS_Bluetooth::Client::run, &c);

    // button input object
    Button_Input buttons;

    // map increase and decrease buttons to wiring pi pins 8 and 9
    buttons.add_function_mapping(Function_Mapping(std::function<void()>(increase_po2), 8, 1));
    buttons.add_function_mapping(Function_Mapping(std::function<void()>(decrease_po2), 9, 1));

    // spawn button thread
    std::thread button_thread(&Button_Input::run, &buttons);

    // write po2 data here
    uint32_t buffer[PACKET_SIZE]{0};

    std::cout << "Transmitting " << (SAMPLE_RATE * SECONDS) << " generated PO2 samples to " << argv[1] << "..." <<  std::endl;

    for (int round = 0; round < (SAMPLE_RATE * SECONDS) / PACKET_SIZE; round++)
    {
        for(int i = 0; i < PACKET_SIZE; i++) {
            buffer[i] = mock_sensor.get();
            // std::cout << mock_sensor.string_get() << std::endl;

            // 1000000 microseconds per second divided by hz gives the total time in between samples. Not perfect but works.
            usleep(1000000 / SAMPLE_RATE);
        }
        c.push(buffer, PACKET_SIZE);
    }

    std::cout << "Finished sending." << std::endl;

    // end bluetooth connection and thread
    c.quit();
    client.join();

    // end button thread
    buttons.quit();
    button_thread.join();

    return 0;
}