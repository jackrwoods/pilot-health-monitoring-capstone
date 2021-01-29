// g++ sql.cpp -lmariadb -o test.out
#include <iostream>

#include "./sql_con.hpp"

// small program to demonstrate that the SQL_Connection object works as intended
int main()
{
	SQL_Connection sql;
	std::vector<Sample> ins;
	for (int i = 0; i < 64; i++)
		ins.push_back(Sample(static_cast<po2_sample>(i), static_cast<optical_sample>(i)));

	std::cout << "Expect three 0's:" << std::endl;
	std::cout << sql.insert_samples(ins) << std::endl;
	std::cout << sql.insert_sample({0, 0}) << std::endl;
	std::cout << sql.insert_state(Pilot_State::STRESSED) << std::endl;
}