#pragma once

#include <cstdint>

using po2_sample = uint32_t;
using optical_sample = uint32_t;

struct Sample {
    po2_sample po2;
    optical_sample optical;
};

enum Pilot_State {
    UNSTRESSED,
    STRESSED
};