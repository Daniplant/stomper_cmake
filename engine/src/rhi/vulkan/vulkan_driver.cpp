#include "vulkan_driver.hpp"

#include <map>
#include <format>
#include <unordered_set>
#include <SDL3/SDL_vulkan.h>

#if defined(SDL_PLATFORM_WIN32)
using namespace Microsoft::WRL;

static std::string GetHResultString(HRESULT hr) {
    _com_error error(hr);
    return error.ErrorMessage();
}
#endif

namespace core::rhi
{
    static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType, 
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
        switch (messageSeverity) {
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
                RHI_DEBUG("{}", pCallbackData->pMessage);
            break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
                RHI_INFO("{}", pCallbackData->pMessage);
            break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
                RHI_WARN("{}", pCallbackData->pMessage);
            break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
                RHI_ERROR("{}", pCallbackData->pMessage);
            break;
            default:
                break;
        };
        return VK_FALSE;
    }
    
#pragma region VulkanFence

    bool VulkanFence::wait() {
       if (auto result = vkWaitForFences(device, 1, &handle, true, SDL_MAX_UINT64); result != VK_SUCCESS) {
           RHI_ERROR("Failed to wait for Vulkan fence: {}", string_VkResult(result));
           return false;
       }
       return true;
    }

    bool VulkanFence::is_signaled() {
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

#pragma endregion

#pragma region VulkanDevice
    VulkanDevice::VulkanDevice(DeviceLUID luid, bool debug) : m_debug(debug)
    {
        m_submit_counter.store(0);

        // Instance creation
        {
            VkDebugUtilsMessengerCreateInfoEXT debug_create_info {
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
                .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
                .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
                .pfnUserCallback = debug_callback,
                .pUserData = nullptr
            };

            if (auto result = volkInitialize(); result != VK_SUCCESS) {
                throw std::runtime_error(std::format("Failed to initialize volk: {}", string_VkResult(result)));
            }

            if (auto result = query_instance_exts(); result != VK_SUCCESS) {
                throw std::runtime_error(std::format("Failed to query available Vulkan instance extensions: {}", string_VkResult(result)));
            }

            // Here we query for the instance extensions we need
            {
                u32 count;
                auto sdl_exts = SDL_Vulkan_GetInstanceExtensions(&count);
                if (!sdl_exts) {
                    throw std::runtime_error(std::format("Failed to query for mandatory Vulkan instance extensions: {}", SDL_GetError()));
                }

                for (int i = 0; i < count; i++) {
                    m_enabled_instance_exts.push_back(sdl_exts[i]);
                }

                if (const char* error; !supports_instance_extensions(m_enabled_instance_exts,&error)) {
                    throw std::runtime_error(std::format("System doesn't support the required Vulkan instance extension {}", error));
                }
                
                m_has_colorspace_ext = supports_instance_extension(VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME);
                if(m_has_colorspace_ext){
                    m_enabled_instance_exts.push_back(VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME);
                } 
            }

            if (m_debug) {
                m_debug = query_validation_layers() && supports_validation_layer("VK_LAYER_KHRONOS_validation") && supports_instance_extension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
                if (m_debug) {
                    m_enabled_layers.push_back("VK_LAYER_KHRONOS_validation");
                    m_enabled_instance_exts.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
                    RHI_INFO("Debug mode enabled, expect some performance loss");
                }
                else {
                    RHI_ERROR("Debug mode requested but failed to enable Vulkan validation layers. Disabling debug");
                }
            }

            VkApplicationInfo app_info {
                .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                .pNext = nullptr,
                .pApplicationName = "",
                .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
                .pEngineName = "Stomper",
                .engineVersion = VK_MAKE_VERSION(1, 0, 0),
                .apiVersion = VK_API_VERSION_1_2
            };

            VkInstanceCreateInfo instance_info {
                .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                .pNext = m_debug ? &debug_create_info : nullptr,
#if defined(SDL_PLATFORM_APPLE)
                .flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR,
#endif
                .pApplicationInfo = &app_info,
                .enabledLayerCount = m_debug ? 0 : (u32)m_enabled_layers.size(),
                .ppEnabledLayerNames = m_enabled_layers.data(),
                .enabledExtensionCount = (u32)m_enabled_instance_exts.size(),
                .ppEnabledExtensionNames = m_enabled_instance_exts.data()
            };

            if (auto result = vkCreateInstance(&instance_info, nullptr, &m_instance); result != VK_SUCCESS) {
                throw std::runtime_error(std::format("Failed to create Vulkan instance: {}", string_VkResult(result)));
            }
            
            volkLoadInstance(m_instance);

            if (m_debug) {
                if (auto result = vkCreateDebugUtilsMessengerEXT(m_instance, &debug_create_info, nullptr, &m_debug_messenger); result != VK_SUCCESS) {
                    m_debug = false;
                    RHI_ERROR("Failed to create Vulkan debug messenger: {}", string_VkResult(result));
                }
            }
        }

        // Device creation
        { 
            auto find_gfx_queue = [&](VkPhysicalDevice physicalDevice) -> u32 {
                SDL_Window* window = SDL_CreateWindow("", 100, 100, SDL_WINDOW_HIDDEN | SDL_WINDOW_VULKAN);

                u32 queue_family_count = 0;
                vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queue_family_count, nullptr);
                std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
                vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queue_family_count, queue_families.data());

                u8 i = 0;
                for (const auto& queue_family : queue_families) {
                    if (queue_family.queueFlags & (VK_QUEUE_GRAPHICS_BIT)) {
                        if (SDL_Vulkan_GetPresentationSupport(m_instance, physicalDevice, i)) {
                            SDL_DestroyWindow(window);
                            return i;
                        }
                    }
                    i++;
                }
                SDL_DestroyWindow(window);
                return VK_QUEUE_FAMILY_IGNORED;
            };

            auto find_dedicated_cmp_queue = [&](VkPhysicalDevice physicalDevice) -> u32 {
                u32 queue_family_count = 0;
                vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queue_family_count, nullptr);
                std::vector<VkQueueFamilyProperties> queueFamilies(queue_family_count);
                vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queue_family_count, queueFamilies.data());

                for (u32 i = 0; i < queue_family_count; ++i) {
                    auto& queue_family = queueFamilies[i];
                    if (queue_family.queueCount > 0 &&
                          queue_family.queueFlags & VK_QUEUE_COMPUTE_BIT &&
                        !(queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
                        return i;
                    }
                }
                return VK_QUEUE_FAMILY_IGNORED;
            };

            auto find_dedicated_trs_queue = [&](VkPhysicalDevice physicalDevice) -> u32 {
                u32 queue_family_count = 0;
                vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queue_family_count, nullptr);
                std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
                vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queue_family_count, queue_families.data());

                for (u32 i = 0; i < queue_family_count; ++i) {
                    auto& queue_family = queue_families[i];
                    if (queue_family.queueCount > 0 &&
                        queue_family.queueFlags & VK_QUEUE_TRANSFER_BIT &&
                        !(queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
                        !(queue_family.queueFlags & VK_QUEUE_COMPUTE_BIT)) {
                        return i;
                    }
                }
                return VK_QUEUE_FAMILY_IGNORED;
            };

            auto check_device_properties = [&](VkPhysicalDevice physicalDevice) -> bool {
                
                if (auto result = query_device_exts(physicalDevice); result != VK_SUCCESS) {
                    RHI_ERROR("Failed to query Vulkan physical device extensions: {}", string_VkResult(result));
                    return false;
                }

                m_rebar = false;
                m_has_memory_budget = false;
                
                m_memory_size = 0;

                m_physical_device_features = {};
                m_physical_device_features11 = {};
                m_physical_device_features12 = {};
                m_physical_device_syncronization2_features_khr = {};
                m_physical_device_dynamic_rendering_features_khr = {};
                m_physical_device_pageable_memory_ext = {};

                m_physical_device_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
                m_physical_device_features11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
                m_physical_device_features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
                m_physical_device_syncronization2_features_khr.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
                m_physical_device_dynamic_rendering_features_khr.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
                m_physical_device_pageable_memory_ext.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PAGEABLE_DEVICE_LOCAL_MEMORY_FEATURES_EXT;

                m_physical_device_features.pNext = &m_physical_device_features11;
                m_physical_device_features11.pNext = &m_physical_device_features12;
                m_physical_device_features12.pNext = &m_physical_device_syncronization2_features_khr;
                m_physical_device_syncronization2_features_khr.pNext   = &m_physical_device_dynamic_rendering_features_khr;
                m_physical_device_dynamic_rendering_features_khr.pNext = &m_physical_device_pageable_memory_ext;
                m_physical_device_pageable_memory_ext.pNext = nullptr;

                m_physical_device_props = {};
                m_physical_device_props11 = {};
                m_physical_device_props12 = {};
                m_depth_stencil_resolve_props = {};

                m_physical_device_props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
                m_physical_device_props11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES;
                m_physical_device_props12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES;
                m_depth_stencil_resolve_props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_STENCIL_RESOLVE_PROPERTIES;

                m_physical_device_props.pNext = &m_physical_device_props11;
                m_physical_device_props11.pNext = &m_physical_device_props12;
                m_physical_device_props12.pNext = &m_depth_stencil_resolve_props;
                m_depth_stencil_resolve_props.pNext = nullptr;

                vkGetPhysicalDeviceFeatures2(physicalDevice, &m_physical_device_features);
                vkGetPhysicalDeviceProperties2(physicalDevice, &m_physical_device_props);

                if (!supports_device_extensions(m_enabled_device_exts)) {
                    return false;
                }
                
                m_has_pageable_memory = supports_device_extensions({ VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME, VK_EXT_PAGEABLE_DEVICE_LOCAL_MEMORY_EXTENSION_NAME }) && 
                    m_physical_device_pageable_memory_ext.pageableDeviceLocalMemory;

                // If you don't have these, sorry but the device is unsupported
                if (find_gfx_queue(physicalDevice) == VK_QUEUE_FAMILY_IGNORED 
                    || !m_physical_device_features.features.samplerAnisotropy
                    || !m_physical_device_features.features.multiDrawIndirect 
                    || !m_physical_device_features11.multiview
                    || !m_physical_device_features12.descriptorIndexing 
                    //|| !m_physical_device_features12.drawIndirectCount
                    || !m_physical_device_features12.bufferDeviceAddress
                    || !m_physical_device_features12.descriptorBindingPartiallyBound
                    || !m_physical_device_features12.descriptorBindingUpdateUnusedWhilePending
                    || !m_physical_device_features12.shaderSampledImageArrayNonUniformIndexing
                    || !m_physical_device_features12.shaderStorageImageArrayNonUniformIndexing
                    || !m_physical_device_features12.descriptorBindingSampledImageUpdateAfterBind
                    || !m_physical_device_features12.descriptorBindingStorageImageUpdateAfterBind
                    || !m_physical_device_syncronization2_features_khr.synchronization2
                    || !m_physical_device_dynamic_rendering_features_khr.dynamicRendering) {
                    return false;
                }

                m_has_memory_budget = supports_device_extension(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME);

                // Vulkan memory types and heaps are black magic to me, I hope this is good enough
                VkPhysicalDeviceMemoryProperties memory_props = {};
                vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memory_props);

                // I think this is how you query the VRAM quantity? TODO: Check with rebar disabled if it's still the same quantity or not
                for (uint32_t i = 0; i < memory_props.memoryHeapCount; i++) {
                    if ((memory_props.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) && memory_props.memoryHeaps[i].size > 268435456) {
                        m_memory_size = memory_props.memoryHeaps[i].size;
                        break;
                    }
                }

                // If you have one heap, then you're running on a UMA architecture. The second flag check is technically redundant but oh well
                if (memory_props.memoryHeapCount == 1 && memory_props.memoryHeaps[0].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
                    m_rebar = true;
                }
                else {
                    // If your DEVICE_LOCAL heap has more than 256 MiB of host visible/coherent memory, then ReBAR is supported. Yay :3
                    for (uint32_t i = 0; i < memory_props.memoryTypeCount; i++) {
                        uint32_t mem_mask = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
                        if ((memory_props.memoryTypes[i].propertyFlags & mem_mask) == mem_mask) {
                            if (memory_props.memoryHeaps[memory_props.memoryTypes[i].heapIndex].size > 268435456) {
                                m_rebar = true;
                                break;
                            }
                        }
                    }
                }
                return true;
            };
           
            u32 devices_count = 0;
            std::vector<VkPhysicalDevice> available_devices;

            if (auto result = vkEnumeratePhysicalDevices(m_instance, &devices_count, nullptr); result != VK_SUCCESS) {
                throw std::runtime_error(std::format("Failed to enumerate Vulkan physical devices: {}", string_VkResult(result)));
            }
            if (devices_count == 0) {
                throw std::runtime_error("Failed to find any suitable Vulkan physical devices");
            }

            available_devices.resize(devices_count);

            if (auto result = vkEnumeratePhysicalDevices(m_instance, &devices_count, available_devices.data()); result != VK_SUCCESS) {
                throw std::runtime_error(std::format("Failed to enumerate Vulkan physical devices: {}", string_VkResult(result)));
            }
            
            std::map<u64, VkPhysicalDevice, std::greater<>> physical_device_ranker;
            for (const auto& physical_device : available_devices) { 
                if (!check_device_properties(physical_device)) {
                    continue;
                }

                u64 score = m_memory_size + (m_rebar ? 1000 : 0);
                score += m_physical_device_props.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ? 1000 : 0;
                score += find_dedicated_cmp_queue(physical_device) != VK_QUEUE_FAMILY_IGNORED ? 1000 : 0;
                score += find_dedicated_trs_queue(physical_device) != VK_QUEUE_FAMILY_IGNORED ? 1000 : 0;
                physical_device_ranker.emplace(score, physical_device);
            }
            if (physical_device_ranker.empty()) {
                throw std::runtime_error("Failed to find any suitable Vulkan physical devices");
            }

            //m_physical_device = std::prev((physical_device_ranker.end()))->second;
            m_physical_device = physical_device_ranker.begin()->second;
            check_device_properties(m_physical_device);

            if (m_has_memory_budget) {
                m_enabled_device_exts.push_back(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME);
            }

            if (m_has_pageable_memory) {
                m_enabled_device_exts.push_back(VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME);
                m_enabled_device_exts.push_back(VK_EXT_PAGEABLE_DEVICE_LOCAL_MEMORY_EXTENSION_NAME);
            }

            m_queue_indices[(u32)CommandQueue::kGeneral] = find_gfx_queue(m_physical_device);
            m_queue_indices[(u32)CommandQueue::kCopy] = find_dedicated_trs_queue(m_physical_device);
            m_queue_indices[(u32)CommandQueue::kCompute] = find_dedicated_cmp_queue(m_physical_device);

            // If there are no dedicated compute/transfer queues, find other ones
            {
                u32 queue_family_count = 0;
                vkGetPhysicalDeviceQueueFamilyProperties(m_physical_device, &queue_family_count, nullptr);
                std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
                vkGetPhysicalDeviceQueueFamilyProperties(m_physical_device, &queue_family_count, queue_families.data());

                for (u32 i = 0; i < queue_family_count; ++i) {
                    auto& queueFamily = queue_families[i];
                    
                    if (m_queue_indices[(u32)CommandQueue::kCopy] == VK_QUEUE_FAMILY_IGNORED &&
                        queueFamily.queueCount > 0 &&
                        queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) {;
                        m_queue_indices[(u32)CommandQueue::kCopy] = i;
                        RHI_WARN("Using non-dedicated Vulkan transfer queue");
                    }

                    if (m_queue_indices[(u32)CommandQueue::kCompute] == VK_QUEUE_FAMILY_IGNORED && 
                        queueFamily.queueCount > 0 && 
                        queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) {
                        m_queue_indices[(u32)CommandQueue::kCompute] = i;
                        RHI_WARN("Using non-dedicated Vulkan compute queue");
                    }
                }
            }
            
            std::unordered_set<u32> unique_families = {
                m_queue_indices[(u32)CommandQueue::kGeneral],
                m_queue_indices[(u32)CommandQueue::kCompute], 
                m_queue_indices[(u32)CommandQueue::kCopy] 
            };

            std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
            float queue_priority = 1.0f;
            for (u32 queue_family : unique_families) {
                VkDeviceQueueCreateInfo queue_create_info = {
                    .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                    .queueFamilyIndex = queue_family,
                    .queueCount = 1,
                    .pQueuePriorities = &queue_priority,
                };
                queue_create_infos.push_back(queue_create_info);
            }

            VkDeviceCreateInfo createInfo = { 
                .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                .pNext = &m_physical_device_features,
                .queueCreateInfoCount = (u32)queue_create_infos.size(),
                .pQueueCreateInfos  = queue_create_infos.data(),
                .enabledExtensionCount = (u32)m_enabled_device_exts.size(),
                .ppEnabledExtensionNames = m_enabled_device_exts.data() 
            };
            if (auto result = vkCreateDevice(m_physical_device, &createInfo, nullptr, &m_device); result != VK_SUCCESS) {
                throw std::runtime_error(std::format("Failed to create Vulkan device: {}", string_VkResult(result)));
            }
            
            volkLoadDevice(m_device);

            vkGetDeviceQueue(m_device, m_queue_indices[(u32)CommandQueue::kGeneral], 0, &m_queues[(u32)CommandQueue::kGeneral]);
            vkGetDeviceQueue(m_device, m_queue_indices[(u32)CommandQueue::kCompute], 0, &m_queues[(u32)CommandQueue::kCompute]);
            vkGetDeviceQueue(m_device, m_queue_indices[(u32)CommandQueue::kCopy], 0, &m_queues[(u32)CommandQueue::kCopy]);
            
             m_samplers_count
                = std::min(MAX_SAMPLER_DESCRIPTORS, m_physical_device_props.properties.limits.maxDescriptorSetSamplers);

             m_sampled_images_count
                = std::min(MAX_SAMPLED_IMAGE_DESCRIPTORS, m_physical_device_props12.maxDescriptorSetUpdateAfterBindSampledImages);

             m_storage_images_count
                = std::min(MAX_STORAGE_IMAGE_DESCRIPTORS, m_physical_device_props12.maxDescriptorSetUpdateAfterBindStorageImages);

             m_storage_buffers_count
                = std::min(MAX_STORAGE_BUFFER_DESCRIPTORS, m_physical_device_props.properties.limits.maxDescriptorSetStorageBuffers);

             m_uniform_buffers_count
                = std::min(MAX_STORAGE_BUFFER_DESCRIPTORS, m_physical_device_props.properties.limits.maxDescriptorSetUniformBuffers);
            
#if defined(SDL_PLATFORM_WIN32)
             if (!setup_dxgi()) {
                 throw std::runtime_error("Failed to setup dxgi");
             }
             m_has_dsr = setup_dsr();
#endif
            RHI_INFO("Vulkan RHI created successfully");
            RHI_INFO("Using GPU device {}", get_name());
        }
    }
    
    VulkanDevice::~VulkanDevice() 
    { 
        vkDeviceWaitIdle(m_device);

        for (auto fence : m_fence_pool) {
            vkDestroyFence(m_device, fence->handle, nullptr);
        }

        for (auto& [thread_id, pool_array] : m_cmdpool_pool) {
            for (auto& pool : pool_array) {
                vkDestroyCommandPool(m_device, pool.handle, nullptr);
            }
        }

        vkDestroyDevice(m_device, nullptr);

        if (m_debug) {
           vkDestroyDebugUtilsMessengerEXT(m_instance, m_debug_messenger, nullptr);
        }
        vkDestroyInstance(m_instance, nullptr);
    }

    CommandBuffer* VulkanDevice::begin_commandbuffer(CommandQueue queue) 
    {
        m_cmd_acquire_mtx.lock();
        VulkanCommandBuffer* cmd = fetch_cmdbuffer(std::this_thread::get_id(), queue);
        m_cmd_acquire_mtx.unlock();

        if (!cmd) {
            return nullptr;
        }

        if (auto result = vkResetCommandBuffer(cmd->handle, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT); result != VK_SUCCESS) {
            RHI_ERROR("Failed to reset Vulkan commandbuffer: {}", string_VkResult(result));
            return nullptr;
        }

        VkCommandBufferBeginInfo cmd_begin_info { 
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext = nullptr,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
            .pInheritanceInfo = nullptr,
        };

        if (auto result = vkBeginCommandBuffer(cmd->handle, &cmd_begin_info); result != VK_SUCCESS) {
            RHI_ERROR("Failed to begin Vulkan commandbuffer recording: {}", string_VkResult(result));
            return nullptr;
        }
        return cmd;
    }

    bool VulkanDevice::end_commandbuffer(CommandBuffer* cmd) 
    { 
        auto vulkan_cmd = static_cast<VulkanCommandBuffer*>(cmd);
        if (auto result = vkEndCommandBuffer(vulkan_cmd->handle); result != VK_SUCCESS) {
            RHI_ERROR("Failed to end Vulkan command buffer recording: {}", string_VkResult(result));
            return false;
        }
        return true;
    }

    bool VulkanDevice::submit_commandbuffer(CommandBuffer* cmd) 
    { 
        auto vulkan_cmd = static_cast<VulkanCommandBuffer*>(cmd);
        
        vulkan_cmd->fence = fetch_fence();
        if (!vulkan_cmd) {
            return false;
        }
        vulkan_cmd->autorelease_fence = true;
        vulkan_cmd->fence->completed.store(false);

        VkCommandBufferSubmitInfo cmd_info {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
            .pNext = nullptr,
            .commandBuffer = vulkan_cmd->handle,
            .deviceMask = 0
        };
        
        VkSubmitInfo2KHR submit_info {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2_KHR,
            .pNext = nullptr,
            .commandBufferInfoCount = 1,
            .pCommandBufferInfos = &cmd_info
        };

        if (auto result = vkQueueSubmit2KHR(m_queues[(u32)vulkan_cmd->pool->queue_type], 1, &submit_info, vulkan_cmd->fence->handle);
            result != VK_SUCCESS) {
            RHI_ERROR("Failed to submit Vulkan commandbuffer: {}", string_VkResult(result));
            return false;
        }

        if ((m_submit_counter.fetch_add(1, std::memory_order_relaxed) % GC_CLEANUP_SUBMIT_THRESHOLD) == 0) {
            //cleanup
        }

        return true;
    }

    Fence* VulkanDevice::submit_commandbuffer_fenced(CommandBuffer* cmd) 
    {
        auto vulkan_cmd = static_cast<VulkanCommandBuffer*>(cmd);
        vulkan_cmd->autorelease_fence = false;

        if (!submit_commandbuffer(cmd)) {
            return nullptr;
        }
        return vulkan_cmd->fence;
    }

    void VulkanDevice::release_fence(Fence* fence) 
    { 
        auto vulkan_fence = static_cast<VulkanFence*>(fence);
        vkResetFences(m_device, 1, &vulkan_fence->handle);

        m_fence_pool_mtx.lock();
        m_fence_pool.push_back(vulkan_fence);
        m_fence_pool_mtx.unlock();
    }

    std::string VulkanDevice::get_name() const 
    { 
        return m_physical_device_props.properties.deviceName;
    }

#if defined(SDL_PLATFORM_WIN32)

    bool VulkanDevice::setup_dxgi() 
    { 
        HRESULT hr;
        if (m_debug) {
            ComPtr<ID3D12Debug> d3d1_dbg;
            hr = D3D12GetDebugInterface(IID_PPV_ARGS(d3d1_dbg.ReleaseAndGetAddressOf()));
            if (SUCCEEDED(hr)) {
                d3d1_dbg->EnableDebugLayer();
                RHI_INFO("Debug mode enabled, expect some performance loss");

                ComPtr<ID3D12DeviceRemovedExtendedDataSettings1> d3d12_dred;
                hr = D3D12GetDebugInterface(IID_PPV_ARGS(d3d12_dred.ReleaseAndGetAddressOf()));
                if (SUCCEEDED(hr)) {
                    d3d12_dred->SetAutoBreadcrumbsEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
                    d3d12_dred->SetPageFaultEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
                    d3d12_dred->SetBreadcrumbContextEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
                }
                else {
                    RHI_ERROR("Failed to enable Device Removed Extended Data (DRED): {}", GetHResultString(hr));
                }

                // https://github.com/turanszkij/WickedEngine/blob/39201b7f32ccfb52c19dd450823f5108b7181b71/WickedEngine/wiGraphicsDevice_DX12.cpp#L2280
                ComPtr<IDXGIInfoQueue> dxgi_info_queue;
                hr = DXGIGetDebugInterface1(0, IID_PPV_ARGS(dxgi_info_queue.ReleaseAndGetAddressOf()));
                if (SUCCEEDED(hr)) {
                    DXGI_INFO_QUEUE_MESSAGE_ID hide[] { 80 };
                    DXGI_INFO_QUEUE_FILTER filter {
                            .DenyList {
                                .NumIDs  = static_cast<UINT>(std::size(hide)),
                                .pIDList = hide,
                        },
                    };
                    dxgi_info_queue->AddStorageFilterEntries(DXGI_DEBUG_DXGI, &filter);
                    dxgi_info_queue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
                    dxgi_info_queue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);
                }
                else {
                    m_debug = false;
                    RHI_ERROR("Failed to enable DXGI debug info queue interface: {}", GetHResultString(hr));
                }
            }
            else {
                RHI_ERROR("Failed to enable D3D12 debug layer: {}", GetHResultString(hr));
            }
        }

        hr = CreateDXGIFactory2(m_debug ? DXGI_CREATE_FACTORY_DEBUG : 0u, IID_PPV_ARGS(&m_dxgi_factory));
        if (FAILED(hr)) {
            RHI_INFO("Failed to create DXGI adapter factory: {}", GetHResultString(hr));
            return false;
        }

        {
            BOOL value;
            hr = m_dxgi_factory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &value, sizeof(BOOL));
            if (FAILED(hr)) {
                m_has_tearing = false;
                RHI_ERROR("Failed to check tearing support: {}", GetHResultString(hr));
            }
            else {
                m_has_tearing = true;
            }
        }

        D3D_FEATURE_LEVEL feature_levels[] = {
            D3D_FEATURE_LEVEL_12_2,
            D3D_FEATURE_LEVEL_12_1,
            D3D_FEATURE_LEVEL_12_0,
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0
        };

        hr = m_dxgi_factory->EnumAdapterByLuid(*reinterpret_cast<const LUID*>(m_physical_device_props11.deviceLUID), IID_PPV_ARGS(m_dxgi_adapter.ReleaseAndGetAddressOf()));
        if (FAILED(hr)) {
            RHI_ERROR("Failed to create DXGI adapter factory from LUID: {}", GetHResultString(hr));
            return false;
        }

        for (auto& feature_level : feature_levels) {
            hr = D3D12CreateDevice(m_dxgi_adapter.Get(), feature_level, IID_PPV_ARGS(m_d3d12_device.ReleaseAndGetAddressOf()));
            if (SUCCEEDED(hr)) {
                break;
            }
        }
        if (!m_d3d12_device) {
            RHI_ERROR("Failed to create D3D12 device");
            return false;
        }

        if (m_debug) {
            ComPtr<ID3D12InfoQueue> d3d12_info_queue;
            if (SUCCEEDED(m_d3d12_device.As(&d3d12_info_queue))) {
                d3d12_info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
                d3d12_info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
                std::vector<D3D12_MESSAGE_SEVERITY> enabledSeverities;
                std::vector<D3D12_MESSAGE_ID> disabledMessages;

                enabledSeverities.push_back(D3D12_MESSAGE_SEVERITY_CORRUPTION);
                enabledSeverities.push_back(D3D12_MESSAGE_SEVERITY_ERROR);
                enabledSeverities.push_back(D3D12_MESSAGE_SEVERITY_WARNING);
                enabledSeverities.push_back(D3D12_MESSAGE_SEVERITY_MESSAGE);

                disabledMessages.push_back(D3D12_MESSAGE_ID_DRAW_EMPTY_SCISSOR_RECTANGLE);
                disabledMessages.push_back(D3D12_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS);
                disabledMessages.push_back(D3D12_MESSAGE_ID_HEAP_ADDRESS_RANGE_INTERSECTS_MULTIPLE_BUFFERS);

                D3D12_INFO_QUEUE_FILTER filter { 
                    .AllowList {
                        .NumSeverities = static_cast<UINT>(enabledSeverities.size()),
                        .pSeverityList = enabledSeverities.data(),
                    },
                    .DenyList {
                        .NumIDs  = static_cast<UINT>(disabledMessages.size()),
                        .pIDList = disabledMessages.data(),
                    } 
                };

                d3d12_info_queue->AddStorageFilterEntries(&filter);
            }
            else {
                RHI_ERROR("Failed to create ID3D12InfoQueue: {}", GetHResultString(hr));
            }
        }

        return true;
    }

    bool VulkanDevice::setup_dsr() 
    { 
        HRESULT hr;
        
        hr = D3D12GetInterface(CLSID_D3D12DSRDeviceFactory, IID_PPV_ARGS(m_dsr_factory.GetAddressOf()));
        if (FAILED(hr)) {
            RHI_ERROR("Failed to Get D3D12DSRDeviceFactory factory: {}", GetHResultString(hr));
            return false;
        }

        hr = m_dsr_factory->CreateDSRDevice(m_d3d12_device.Get(),0,IID_PPV_ARGS(m_dsr_device.GetAddressOf()));
        if (FAILED(hr)) {
            RHI_ERROR("Failed to create DSRDevice: {}", GetHResultString(hr));
            return false;
        }

        const u32 dsrVariantCount = m_dsr_device->GetNumSuperResVariants();
        for (u32 currentVariantIndex = 0; currentVariantIndex < dsrVariantCount; currentVariantIndex++)
        {
            DSR_SUPERRES_VARIANT_DESC variantDesc;
            m_dsr_device->GetSuperResVariantDesc(currentVariantIndex, &variantDesc);
            RHI_INFO("Found DSR variant: {}", variantDesc.VariantName);
        }
    }

    bool VulkanDevice::create_dxgi_swapchain(SDL_Window* window) 
    { 
        return false;
    }

#endif

    VkResult VulkanDevice::query_instance_exts() 
    {
        u32 count;

        if (auto result = vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr); result != VK_SUCCESS) {
            return result;
        }

        m_available_instance_exts.resize(count);

        if (auto result = vkEnumerateInstanceExtensionProperties(nullptr, &count, m_available_instance_exts.data()); result != VK_SUCCESS) {
            return result;        
        }

        return VK_SUCCESS;
    }

    VkResult VulkanDevice::query_device_exts(VkPhysicalDevice device) 
    { 
        u32 count;
        if (auto result = vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr); result != VK_SUCCESS) {
            return result;
        }

        m_available_device_exts.resize(count);

        if (auto result = vkEnumerateDeviceExtensionProperties(device, nullptr, &count, m_available_device_exts.data()); result != VK_SUCCESS) {
            return result;
        }

        return VK_SUCCESS;
    }

    bool VulkanDevice::query_validation_layers() 
    {
        u32 count;

        if (auto result = vkEnumerateInstanceLayerProperties(&count, nullptr); result != VK_SUCCESS) {
            RHI_ERROR("Failed to query available Vulkan validation layers: {}", string_VkResult(result));
            return false;
        }
        
        m_available_layers.resize(count);

        if (auto result = vkEnumerateInstanceLayerProperties(&count, m_available_layers.data()); result != VK_SUCCESS) {
            RHI_ERROR("Failed to query available Vulkan validation layers: {}", string_VkResult(result));
            return false;
        }

        return true;
    }

    bool VulkanDevice::supports_validation_layer(const char* layer_name) const 
    {
        for (auto& layer : m_available_layers) {
            if (strcmp(layer.layerName, layer_name) == 0) {
                return true;
            }
        }
        return false;
    };

    bool VulkanDevice::supports_instance_extension(const char* extension_name) const 
    { 
        for (auto& ext : m_available_instance_exts) {
            if (strcmp(ext.extensionName, extension_name) == 0) {
                return true;
            }
        }
        return false;
    }

    bool VulkanDevice::supports_instance_extensions(std::vector<const char*> extension_names, const char** unsupported_ext) const {
        for (auto& ext : extension_names) {
            if (!supports_instance_extension(ext)) {
                if (unsupported_ext) {
                    *unsupported_ext = ext;
                }
                return false;
            }
        }
        return true;
    }
    
    bool VulkanDevice::supports_device_extension(const char* extension_name) const 
    {
        for (auto& ext : m_available_device_exts) {
            if (strcmp(ext.extensionName, extension_name) == 0) {
                return true;
            }
        }
        return false;
    }
    
    bool VulkanDevice::supports_device_extensions(std::vector<const char*> extension_names, const char** unsupported_ext) const 
    {
        for (auto& ext : extension_names) {
            if (!supports_device_extension(ext)) {
                if (unsupported_ext) {
                    *unsupported_ext = ext;
                }
                return false;
            }
        }
        return true;
    }

    VulkanFence* VulkanDevice::fetch_fence() 
    {
        std::lock_guard lock(m_fence_pool_mtx);

        if (m_fence_pool.empty()) {
            auto fence = std::make_unique<VulkanFence>();
            fence->device = m_device;

            VkFenceCreateInfo createInfo { 
                .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, 
                .pNext = nullptr, 
                .flags = 0 
            };
            if (auto result = vkCreateFence(m_device, &createInfo, nullptr, &fence->handle); result != VK_SUCCESS) {
                RHI_ERROR("Failed to create Vulkan fence: {}", string_VkResult(result));
                return nullptr;
            }
            m_fence_pool.push_back(fence.release());
        }

        VulkanFence* fence = m_fence_pool.back();
        m_fence_pool.pop_back();
        return fence;
    }

    VulkanSemaphore* VulkanDevice::fetch_semaphore() 
    { 
        std::lock_guard lock(m_semaphore_pool_mtx);

        if (m_semaphore_pool.empty()) {

            auto semaphore = std::make_unique<VulkanSemaphore>();
            semaphore->signal_value = 0;
            semaphore->current_value = 0;

            VkSemaphoreTypeCreateInfoKHR type_info { 
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
                .pNext = nullptr,
                .semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE_KHR,
                .initialValue = 0 
            };

            VkSemaphoreCreateInfo create_info {
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
                .pNext = nullptr
            };

            if (auto result = vkCreateSemaphore(m_device, &create_info, nullptr, &semaphore->handle); result != VK_SUCCESS) {
                RHI_ERROR("Failed to create Vulkan semaphore: {}", string_VkResult(result));
                return nullptr;
            }

            m_semaphore_pool.push_back(semaphore.release());
        }
    
        VulkanSemaphore* semaphore = m_semaphore_pool.back();
        m_semaphore_pool.pop_back();
        return semaphore;
    }

    VulkanCommandBuffer* VulkanDevice::fetch_cmdbuffer(std::thread::id thread_id, CommandQueue queue) 
    { 
        VulkanCommandPool* cmd_pool = fetch_cmdpool(thread_id, queue);
        if (!cmd_pool) {
            return nullptr;
        }

        if (cmd_pool->free_cmdbuffers.empty()) {
            if (!allocate_cmdbuffer(cmd_pool)) {
                return nullptr;
            }
        }

        VulkanCommandBuffer* cmd = cmd_pool->free_cmdbuffers.back();
        cmd_pool->free_cmdbuffers.pop_back();
        return cmd;
    }

    VulkanCommandPool* VulkanDevice::fetch_cmdpool(std::thread::id thread_id, CommandQueue queue) 
    { 
        if (!m_cmdpool_pool.contains(thread_id)) {

            m_cmdpool_pool[thread_id] = {};

            for (auto& cmdpool : m_cmdpool_pool[thread_id]) {

                cmdpool.queue_type = queue;
                cmdpool.thread_id  = thread_id;

                VkCommandPoolCreateInfo cmdpool_info { 
                    .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                    .queueFamilyIndex = m_queue_indices[(u32)queue] 
                };

                if (auto result = vkCreateCommandPool(m_device, &cmdpool_info, nullptr, &cmdpool.handle); result != VK_SUCCESS) {
                    RHI_ERROR("Failed to create Vulkan command pool: {}", string_VkResult(result));
                    return nullptr;
                }

                if (!allocate_cmdbuffer(&m_cmdpool_pool[thread_id][(u32)queue])) {
                    return nullptr;
                }
            }
        }

        return &m_cmdpool_pool.at(thread_id)[(u32)queue];
    }

    bool VulkanDevice::allocate_cmdbuffer(VulkanCommandPool* cmdpool) 
    {
        auto cmdbuffer = std::make_unique<VulkanCommandBuffer>();
        
        VkCommandBufferAllocateInfo alloc_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = nullptr,
            .commandPool = cmdpool->handle,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1
        };

        if (auto result = vkAllocateCommandBuffers(m_device, &alloc_info, &cmdbuffer->handle); result != VK_SUCCESS) {
            RHI_ERROR("Failed to allocate Vulkan commandbuffer: {}", string_VkResult(result));
            return false;
        }

        cmdbuffer->pool = cmdpool;
        cmdbuffer->pool->free_cmdbuffers.emplace_back(cmdbuffer.release());
        return true;
    }

#pragma endregion

}
