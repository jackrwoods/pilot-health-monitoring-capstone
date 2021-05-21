#include <iostream>
#include <chrono>
#include <assert.h>

#include "./sql_con.hpp"

// small program to demonstrate that the SQL_Connection object works as intended
int main()
{
	SQL_Connection sql;
	std::vector<Sample> sample_ins;
	std::vector<Pilot_State> state_ins;
	for (int i = 0; i < 64; i++) {
		sample_ins.push_back(Sample(static_cast<po2_sample>(i), static_cast<optical_sample>(i)));
		state_ins.push_back(static_cast<Pilot_State>(i%2));
	}

	// at each assert that there were no errors

	// insert 64 samples
	auto start = std::chrono::high_resolution_clock::now();
	assert(0 == sql.insert_samples(sample_ins));
	auto end = std::chrono::high_resolution_clock::now();
	auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() / 64;
	float hz = 1000000000.0 / nanos;
	std::cout << "Inserted 64 Samples in " << nanos << " nanoseconds (max data transfer: " << hz << " hz)" << std::endl;

	// single sample insert
	start = std::chrono::high_resolution_clock::now();
	assert(0 == sql.insert_sample({0, 0}));
	end = std::chrono::high_resolution_clock::now();
	nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
	hz = 1000000000.0 / nanos;
	std::cout << "Inserted 1 Sample in " << nanos << " nanoseconds (max data transfer: " << hz << " hz)" << std::endl;

	// insert 64 states
	start = std::chrono::high_resolution_clock::now();
	assert(0 == sql.insert_states(state_ins));
	end = std::chrono::high_resolution_clock::now();
	nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() / 64;
	hz = 1000000000.0 / nanos;
	std::cout << "Inserted 64 states in " << nanos << " nanoseconds (max data transfer: " << hz << " hz)" << std::endl;

	// single state insert
	start = std::chrono::high_resolution_clock::now();
	assert(0 == sql.insert_state(Pilot_State::STRESSED));
	end = std::chrono::high_resolution_clock::now();
	nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
	hz = 1000000000.0 / nanos;
	std::cout << "Inserted 1 state in " << nanos << " nanoseconds (max data transfer: " << hz << " hz)" << std::endl;

	return 0;
}