#pragma once
#include "engine/common.hpp"
#include <SDL3/SDL_gpu.h>
#include <vector>

namespace core::assets
{
    struct TextureHeader
    {
        u32 width, height, depth, depth_or_layers, num_mips;
        SDL_GPUTextureType type;
        SDL_GPUTextureFormat format;
        std::vector<u64> sizes;
    };
} // namespace core::assets
