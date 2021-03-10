#include <iostream>
#include <thread>

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

int main()
{
    std::cout << "Test data I/O block\n";

    Data_IO<IO_TYPES::Sample> dio(ds);

    int init_bpm = ds.get_ece_bpm();
    int init_po2 = ds.get_ece_po2();

    std::cout << "Initial ECE BPM value: " << init_bpm << std::endl;
    std::cout << "Initial ECE PO2 value: " << init_po2 << std::endl;

    std::thread th0(thread_ex);
    std::thread th_dio(&Data_IO<IO_TYPES::Sample>::run, std::ref(dio));
    std::cout << "Data I/O block is receiving..." << std::endl;

    std::cin.getline(nullptr, 0);
    dio.quit();
    th_dio.join();

    thread_run = false;
    th0.join();
    // test that bpm and po2 values were received
    std::cout << "Current ECE BPM value: " << ds.get_ece_bpm() << std::endl;
    std::cout << "Current ECE PO2 value: " << ds.get_ece_po2() << std::endl;

    dio.quit();
    return 0;
}