#pragma once
#include "../sys_rhi.hpp"
#include "engine/common.hpp"
#include "engine/rhi/rhi.hpp"

#include "volk.h"
#include <array>
#include <string.h>
#include <SDL3/SDL_platform_defines.h>

#if defined(SDL_PLATFORM_WIN32)
#include <wrl.h>
#include <windows.h>
#include <dxgi1_6.h>
#include <directx/d3d12.h>
#include <vulkan/vulkan_win32.h>
#endif

namespace core::rhi
{
    class VulkanDevice final : public Device
    {
    public:
        VulkanDevice(DeviceLUID luid, bool debug);
        ~VulkanDevice() override;

        bool supports_advanced_sync() override {return false;} // Vulkan cannot sync command buffers across queues submits
        
        bool create_swapchain(SDL_Window* window) override {return false;}
        void destroy_swapchain(SDL_Window* window) override { }
 
        CommandBuffer* begin_commandbuffer(CommandQueue queue) override { return nullptr; }
        void end_commandbuffer(CommandBuffer* cmd) override {}
        
        bool submit_commandbuffers(std::span<CommandBuffer*> cmds) override { return false;}

        std::string get_name() const override { return m_physical_device_props.properties.deviceName;}

    private:
        VkResult query_instance_exts();
        VkResult query_device_exts(VkPhysicalDevice device);
        bool query_validation_layers();

        bool supports_validation_layer(const char* layer_name) const;
        bool supports_instance_extension(const char* extension_name) const;
        bool supports_instance_extensions(std::vector<const char*> extension_names, const char** unsupported_ext = nullptr) const;
        bool supports_device_extension(const char* extension_name) const;
        bool supports_device_extensions(std::vector<const char*> extension_names, const char** unsupported_ext = nullptr) const;

    private:
        bool m_debug;
        bool m_rebar;
        bool m_using_dxgi;
        bool m_single_queue;
        bool m_has_memory_budget;
        bool m_has_colorspace_ext;

        u64 m_memory_size;
        u64 m_samplers_count;
        u64 m_sampled_images_count;
        u64 m_storage_images_count;
        u64 m_storage_buffers_count;
        u64 m_uniform_buffers_count;
        
        VkInstance m_instance;
        
        VkDevice m_device;
        VkPhysicalDevice m_physical_device;
        
        std::array<VkQueue, (u32)CommandQueue::kMax> m_queues;
        std::array<u32, (u32)CommandQueue::kMax> m_queue_indices;

        VkDebugUtilsMessengerEXT m_debug_messenger;

        std::vector<const char*> m_enabled_layers;
        std::vector<const char*> m_enabled_instance_exts = {
            VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME,
            VK_KHR_EXTERNAL_FENCE_CAPABILITIES_EXTENSION_NAME,
            VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME
        };
        
        std::vector<const char*> m_enabled_device_exts = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            VK_EXT_MEMORY_BUDGET_EXTENSION_NAME,
            VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
            VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
#if defined(SDL_PLATFORM_WIN32)
            VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME,
            VK_KHR_EXTERNAL_FENCE_WIN32_EXTENSION_NAME,
            VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME,
#elif defined(SDL_PLATFORM_LINUX)
            VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME,
            VK_KHR_EXTERNAL_FENCE_FD_EXTENSION_NAME,
            VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME,
#endif
        };

        std::vector<VkLayerProperties> m_available_layers;
        std::vector<VkExtensionProperties> m_available_device_exts;
        std::vector<VkExtensionProperties> m_available_instance_exts;
        
        VkPhysicalDeviceFeatures2 m_physical_device_features;
        VkPhysicalDeviceVulkan11Features m_physical_device_features11;
        VkPhysicalDeviceVulkan12Features m_physical_device_features12;
        VkPhysicalDeviceSynchronization2FeaturesKHR m_physical_device_syncronization2_features_khr;
        VkPhysicalDeviceDynamicRenderingFeaturesKHR m_physical_device_dynamic_rendering_features_khr;

        VkPhysicalDeviceProperties2 m_physical_device_props;
        VkPhysicalDeviceVulkan11Properties m_physical_device_props11;
        VkPhysicalDeviceVulkan12Properties m_physical_device_props12;
        VkPhysicalDeviceDescriptorIndexingProperties m_descriptor_indexing_props;
        VkPhysicalDeviceDepthStencilResolveProperties m_depth_stencil_resolve_props;

        #if defined(SDL_PLATFORM_WIN32)
        Microsoft::WRL::ComPtr<ID3D12Device1> m_d3d_device;
        Microsoft::WRL::ComPtr<IDXGIFactory4> m_dxgi_factory;
        Microsoft::WRL::ComPtr<IDXGIAdapter4> m_dxgi_adapter; 
        #endif
    };
}
