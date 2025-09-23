#pragma once
#include "engine/common.hpp"
#include <SDL3/SDL_gpu.h>
#include <cereal/cereal.hpp>
#include <cereal/types/array.hpp>
#include <cereal/types/string.hpp>

namespace core::assets
{
    struct RenderConfig
    {
        bool low_latency               = true;
        SDL_GPUSampleCount multisample = SDL_GPU_SAMPLECOUNT_4;
        std::string device_luid        = "";

        template <class Archive> void serialize(Archive& archive, const u32 version) {
            archive(CEREAL_NVP(low_latency), CEREAL_NVP(multisample), CEREAL_NVP(device_luid));
        }
    };

    struct GameConfig
    {
        RenderConfig render_config = {};

        template <class Archive> void serialize(Archive& archive, const u32 version) { archive(CEREAL_NVP(render_config)); }
    };
} // namespace core::assets
