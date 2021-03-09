#include <iostream>
#include <thread>

#include "./data_io.hpp"
#include "./io_types.hpp"  // temporary access to sample types for testing only

int main()
{

    std::cout << "Test data I/O block\n";
 
    Data_IO<Sample> dio;
 
    std::thread th0 (&Data_IO<Sample>::run, std::ref(dio));
 
    std::cin.getline(nullptr, 0);
 
    dio.quit();
 
    th0.join();
    return 0;
}