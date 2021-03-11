#include <iostream>
#include <thread>
#include <chrono>

#include "./data_io.hpp"
#include "./io_types.hpp" // temporary access to sample types for testing only

#include "../data-store/ds_data_store.hpp" // use data store for tests

Data_Store<IO_TYPES::Sample, 256> ds;

bool thread_run{true};

void thread_ex()
{
    // count received samples
    int total_samples{0};

    ds.register_reader_thread();
    while (thread_run)
    {
        if (ds.available_samples() > 0)
        {
            total_samples += ds.vec().size();
        }
    }
    std::cout << total_samples * 2 << " total received samples.\n";
}

int main(int argc, char *argv[])
{
    std::cout << "Data IO block tests - Recieve Samples and send Pilot states.\n";

    if (argc != 2)
    {
        std::cerr << "Exactly one command line argument required: ./test_client.out [blueooth address]" << std::endl;
        return 1;
    }

    std::string bt_address = std::string(argv[1]);

    Data_IO<IO_TYPES::Sample> dio(ds);
    dio.open_server();

    if (!dio.connect(bt_address))
    {
        std::cerr << "An error occurred connecting to " << bt_address << std::endl;
        return 1;
    }

    int init_bpm = ds.get_ece_bpm();
    int init_po2 = ds.get_ece_po2();

    std::cout << "Initial ECE BPM value: " << init_bpm << std::endl;
    std::cout << "Initial ECE PO2 value: " << init_po2 << std::endl;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Receive Samples and ECE measurements.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    std::thread th0(thread_ex);

    std::thread th_dio(&Data_IO<IO_TYPES::Sample>::run, std::ref(dio));
    std::cout << "Data I/O block is receiving..." << std::endl;

    // control when program stops
    std::cout << "Press enter when all samples have been transmitted.\n";
    std::cin.getline(nullptr, 0);

    thread_run = false;
    th0.join();
    // test that bpm and po2 values were received
    std::cout << "Current ECE BPM value: " << ds.get_ece_bpm() << std::endl;
    std::cout << "Current ECE PO2 value: " << ds.get_ece_po2() << std::endl;

    // Pilot state tests
    std::cout << "Press enter to begin Pilot state transmission tests. (2)\n";
    std::cin.getline(nullptr, 0);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Send Pilot State Samples
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // transmit 64 pilot states over bluetooth

    std::cout << "Transmitting Pilot States\n";

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < 64; i++)
        dio.send_pilot_state(static_cast<IO_TYPES::Pilot_State>(i % 2));

    auto end = std::chrono::high_resolution_clock::now();
    auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() / 64;
    float hz = 1000000000.0 / nanos;

    std::cout << "Transmitted Pilot states at " << hz << " hertz. \n";

    std::cout << "Press enter to complete program." << std::endl;
    std::cin.getline(nullptr, 0);

    dio.quit();
    th_dio.join();
    return 0;
}