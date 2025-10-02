#pragma once
#include "../sys_rhi.hpp"
#include "engine/common.hpp"
#include "engine/rhi/rhi.hpp"

#include "volk.h"
#include <vulkan/vk_enum_string_helper.h>

#include <array>
#include <SDL3/SDL_platform_defines.h>

#if defined(SDL_PLATFORM_WIN32)
#define NOMINMAX
#include <wrl.h>
#include <comdef.h>
#include <windows.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include <directx/d3d12.h>
#include <directx/directsr.h>
#include <vulkan/vulkan_win32.h>
#endif

namespace core::rhi
{
    struct VulkanCommandPool;

    struct VulkanFence : public Fence
    {
        VulkanFence() = default;
        ~VulkanFence() override = default;

        bool wait() override 
        { 
            if (auto result = vkWaitForFences(device, 1, &handle, true, SDL_MAX_UINT64); result != VK_SUCCESS) {
                RHI_ERROR("Failed to wait for Vulkan fence: {}", string_VkResult(result));
                return false;
            }
            return true;
        }

        bool is_signaled() override
        { 
            auto result = vkGetFenceStatus(device, handle);
            if (result == VK_SUCCESS) {
                return true;
            }
            else if (result == VK_NOT_READY) {
                return false;
            }
            else {
                RHI_ERROR("Failed to query Vulkan fence status: {}", string_VkResult(result));
                return false;
            }
        }

        VkFence handle;
        VkDevice device;
        std::atomic_bool completed;
    };

    struct VulkanCommandBuffer : public CommandBuffer
    {
        VulkanCommandBuffer() = default;
        ~VulkanCommandBuffer() override = default;

        VulkanCommandPool* pool;

        VkCommandBuffer handle;

        VulkanFence* fence;
        bool autorelease_fence;
    };

    struct VulkanCommandPool
    {
        CommandQueue queue_type;

        std::thread::id thread_id;

        VkCommandPool handle;
        std::vector<VulkanCommandBuffer*> free_cmdbuffers;
    };

    class VulkanDevice final : public Device
    {
    public:
        VulkanDevice(DeviceLUID luid, bool debug);
        ~VulkanDevice() override;

        bool supports_advanced_sync() override { return false; }
        
        bool create_swapchain(SDL_Window* window) override { throw std::runtime_error("Not yet implemented"); }
        void destroy_swapchain(SDL_Window* window) override { throw std::runtime_error("Not yet implemented"); }
 
        CommandBuffer* begin_commandbuffer(CommandQueue queue) override;
        bool end_commandbuffer(CommandBuffer* cmd) override;
        
        bool submit_commandbuffer(CommandBuffer* cmds) override;
        Fence* submit_commandbuffer_fenced(CommandBuffer* cmds) override;
        
        void destroy_fence(Fence* fence) override;

        std::string get_name() const override;

    private:
#if defined(SDL_PLATFORM_WIN32)
        bool setup_dxgi();
        bool setup_dsr();
#endif

        VkResult query_instance_exts();
        VkResult query_device_exts(VkPhysicalDevice device);
        bool query_validation_layers();

        bool supports_validation_layer(const char* layer_name) const;
        bool supports_instance_extension(const char* extension_name) const;
        bool supports_instance_extensions(std::vector<const char*> extension_names, const char** unsupported_ext = nullptr) const;
        bool supports_device_extension(const char* extension_name) const;
        bool supports_device_extensions(std::vector<const char*> extension_names, const char** unsupported_ext = nullptr) const;

        VulkanFence* fetch_fence();
        VulkanCommandBuffer* fetch_cmdbuffer(std::thread::id thread, CommandQueue queue);

        VulkanCommandPool* fetch_cmdpool(std::thread::id thread, CommandQueue queue);
        bool allocate_cmdbuffer(VulkanCommandPool* cmdpool);

    private:
        bool m_debug;
        bool m_rebar;
        bool m_using_dxgi;
        bool m_has_tearing;
        bool m_has_memory_budget;
        bool m_has_colorspace_ext;
        bool m_has_pageable_memory;

        u64 m_memory_size;
        u64 m_samplers_count;
        u64 m_sampled_images_count;
        u64 m_storage_images_count;
        u64 m_storage_buffers_count;
        u64 m_uniform_buffers_count;
        
        std::atomic_uint64_t m_submit_counter;

        VkInstance m_instance;
        
        VkDevice m_device;
        VkPhysicalDevice m_physical_device;
        VkDebugUtilsMessengerEXT m_debug_messenger;

        std::mutex m_fence_pool_mtx;
        std::mutex m_cmd_submit_mtx;
        std::mutex m_cmd_acquire_mtx;
        
        std::array<VkQueue, (u32)CommandQueue::kMax> m_queues;
        std::array<u32, (u32)CommandQueue::kMax> m_queue_indices;
        
        std::vector<VulkanFence*> m_fence_pool;

        std::vector<VulkanCommandBuffer*> m_submitted_cmdbuffers;
        std::unordered_map<std::thread::id, std::array<VulkanCommandPool, (u32)CommandQueue::kMax>> m_cmdpool_pool;

        std::vector<const char*> m_enabled_layers;
        std::vector<const char*> m_enabled_instance_exts;
        
        std::vector<const char*> m_enabled_device_exts = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            VK_EXT_MEMORY_BUDGET_EXTENSION_NAME,
            VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
            VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
#if defined(SDL_PLATFORM_WIN32)
            VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME,
            VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME,
#elif defined(SDL_PLATFORM_LINUX)
            VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME,
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
        VkPhysicalDevicePageableDeviceLocalMemoryFeaturesEXT m_physical_device_pageable_memory_ext;

        VkPhysicalDeviceProperties2 m_physical_device_props;
        VkPhysicalDeviceVulkan11Properties m_physical_device_props11;
        VkPhysicalDeviceVulkan12Properties m_physical_device_props12;
        VkPhysicalDeviceDescriptorIndexingProperties m_descriptor_indexing_props;
        VkPhysicalDeviceDepthStencilResolveProperties m_depth_stencil_resolve_props;
        
        #if defined(SDL_PLATFORM_WIN32)
        Microsoft::WRL::ComPtr<ID3D12Device1> m_d3d12_device;
        Microsoft::WRL::ComPtr<IDXGIFactory5> m_dxgi_factory;
        Microsoft::WRL::ComPtr<IDXGIAdapter4> m_dxgi_adapter; 

        Microsoft::WRL::ComPtr<IDSRDevice> m_dsr_device;
        Microsoft::WRL::ComPtr<ID3D12DSRDeviceFactory> m_dsr_factory;
    #endif
    };
}
