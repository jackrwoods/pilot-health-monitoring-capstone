// g++ sql.cpp -lmariadb -o test.out
#include <iostream>
#include <chrono>

#include "./sql_con.hpp"

// small program to demonstrate that the SQL_Connection object works as intended
int main()
{
	SQL_Connection sql;
	std::vector<Sample> ins;
	for (int i = 0; i < 64; i++)
		ins.push_back(Sample(static_cast<po2_sample>(i), static_cast<optical_sample>(i)));

	std::cout << "Expect three 0's:" << std::endl;

	// begin clock
	auto start = std::chrono::high_resolution_clock::now();

	// insert 64 samples
	std::cout << sql.insert_samples(ins) << std::endl;

	auto end = std::chrono::high_resolution_clock::now();
	auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() / 64;
	float hz = 1000000000.0 / nanos;

	std::cout << "Inserted 64 Samples in " << nanos << " nanoseconds (max data transfer: " << hz << " hz)" << std::endl;

	std::cout << sql.insert_sample({0, 0}) << std::endl;
	std::cout << sql.insert_state(Pilot_State::STRESSED) << std::endl;
}