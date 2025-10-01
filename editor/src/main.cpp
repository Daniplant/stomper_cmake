#include <cereal/archives/json.hpp>
#include <cereal/archives/xml.hpp>
#include <engine/assets/configs.hpp>
#include <engine/rhi/rhi.hpp>
#include <spdlog/spdlog.h>

using namespace core::rhi;

class TestClass
{

};

int main() {
    spdlog::set_level(spdlog::level::debug);

    SDL_Window* window = SDL_CreateWindow("", 800, 800, SDL_WINDOW_VULKAN | SDL_WINDOW_HIDDEN);
    Device* device;
#if defined(SDL_PLATFORM_WINDOWS) || defined(SDL_PLATFORM_LINUX)
    device = create_vulkan_device(NullDeviceLUID, true);
#elif defined(SDL_PLATFORM_APPLE)
    device = create_metal_device(true);
    //device = create_vulkan_device(NullDeviceLUID, true);
    auto cmd = device->begin_commandbuffer(CommandQueue::kGeneral);
    device->end_commandbuffer(cmd);
    auto fence = device->submit_commandbuffer_fenced(cmd);
    
    while(!fence->is_signaled()){
        spdlog::info("Waiting for completition...");
    }
    spdlog::info("Completed!");
    
#endif
    delete device;
}
