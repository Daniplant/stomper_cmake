#include "sys_rhi.hpp"
#include "engine/rhi/rhi.hpp"

#include <memory>
#include <spdlog/spdlog.h>
#include <SDL3/SDL_platform_defines.h>

#if defined(SDL_PLATFORM_WINDOWS) || defined(SDL_PLATFORM_LINUX) || defined(SDL_PLATFORM_APPLE)
#include "vulkan/vulkan_driver.hpp"
#endif

#ifdef SDL_PLATFORM_APPLE
#include "metal/metal_driver_base.hpp"
#endif

extern "C" { __declspec(dllexport) extern const UINT D3D12SDKVersion = 715; }
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = ".\\D3D12\\"; }

namespace core::rhi
{
    Device* create_metal_device(bool debug) {
#ifndef SDL_PLATFORM_APPLE
        spdlog::error("The Metal RHI driver is only supported on Apple devices");
        return nullptr;
#else
        try {
            auto device = make_metal3_device(debug);
            return device.release();
        }
        catch (const std::exception& e) {
            spdlog::error(e.what());
            return nullptr;
        }
#endif
    }

    Device* create_vulkan_device(DeviceLUID luid, bool debug)
    {
#if not defined(SDL_PLATFORM_WINDOWS) && not defined(SDL_PLATFORM_LINUX) && not defined(SDL_PLATFORM_APPLE)
        spdlog::error("The Vulkan RHI driver is only supported on Windows, Linux and Apple devices");
        return nullptr;
#else
        try {
            auto device = std::make_unique<VulkanDevice>(luid, debug);
            return device.release();
        }
        catch (const std::exception& e) {
            spdlog::error(e.what());
            return nullptr;
        }
#endif
    }
} // namespace core::rhi
