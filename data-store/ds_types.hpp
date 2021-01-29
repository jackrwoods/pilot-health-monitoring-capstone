#pragma once

#include <cstdint>
#include <chrono>
#include <iostream>

using po2_sample = uint32_t;
using optical_sample = uint32_t;

struct Sample
{
	po2_sample po2;
	optical_sample optical;
	std::chrono::_V2::system_clock::time_point time_stamp;
	Sample();
	Sample(po2_sample p, optical_sample o);
};

Sample::Sample()
{
	time_stamp = std::chrono::system_clock::now();
};
Sample::Sample(po2_sample p, optical_sample o) : po2(p), optical(o)
{
	time_stamp = std::chrono::system_clock::now();
}

std::ostream &operator<<(std::ostream &os, const Sample s)
{
	os << "Sample recorded at " << s.time_stamp.time_since_epoch().count() << " (po2: " << s.po2 << ", optical: " << s.optical << ")";
	return os;
}

enum Pilot_State
{
	UNSTRESSED,
	STRESSED
};