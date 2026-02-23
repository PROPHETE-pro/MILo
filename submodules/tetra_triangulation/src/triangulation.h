#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>

#include "utils/exception.h"

#if defined(__has_include)
#  if __has_include(<cuda_runtime.h>)
#    include <cuda_runtime.h>
#    define TETRANERF_HAS_CUDA_RUNTIME 1
#  endif
#endif

#ifndef TETRANERF_HAS_CUDA_RUNTIME
struct float3 {
    float x;
    float y;
    float z;
};

struct uint4 {
    std::uint32_t x;
    std::uint32_t y;
    std::uint32_t z;
    std::uint32_t w;
};
#endif

static_assert(sizeof(float3) == sizeof(float) * 3, "float3 must be tightly packed");
static_assert(sizeof(uint4) == sizeof(std::uint32_t) * 4, "uint4 must be tightly packed");

std::vector<uint4> triangulate(size_t num_points, float3* points);