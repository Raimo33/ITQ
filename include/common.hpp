#pragma once

#include <cstdint>
#include <atomic>
#include <array>

#ifndef CACHELINE_SIZE
# define CACHELINE_SIZE 64
#endif

template <size_t N>
concept is_power_of_two = (N > 0) && ((N & (N - 1)) == 0);
