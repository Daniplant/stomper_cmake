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
#if defined(SDL_PLATFORM_WINDOWS)
    device = create_vulkan_device(NullDeviceLUID, true);
#elif defined(SDL_PLATFORM_APPLE)
    //device = create_metal_device(true);
    device = create_vulkan_device(NullDeviceLUID, true);
    spdlog::info(device->get_name());
#endif
    delete device;
}
