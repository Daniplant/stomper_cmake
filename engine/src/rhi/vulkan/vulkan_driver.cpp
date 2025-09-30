#define VOLK_IMPLEMENTATION

#if defined(SDL_PLATFORM_WIN32)
    #define VK_USE_PLATFORM_WIN32_KHR
#endif
#include "volk.h"

#define ENGINE_NAME "Stomper"

#ifndef GAME_VERSION_MAJOR
    #define GAME_VERSION_MAJOR 1
#endif

#ifndef GAME_VERSION_MINOR
    #define GAME_VERSION_MINOR 0
#endif

#ifndef GAME_VERSION_PATCH
    #define GAME_VERSION_PATCH 0
#endif

#ifndef GAME_NAME
    #define GAME_NAME "Unknown stomper game"
#endif

#include "vulkan_driver.hpp"

#include <map>
#include <format>
#include <unordered_set>

#include <SDL3/SDL_vulkan.h>
#include <vulkan/vk_enum_string_helper.h>

#if defined(SDL_PLATFORM_WIN32)
#include <comdef.h>
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
            
    VulkanDevice::VulkanDevice(DeviceLUID luid, bool debug) : m_debug(debug)
    {
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

            // Here we query for the manadatory instance extensions we need
            {
                u32 count;
                auto sdl_exts = SDL_Vulkan_GetInstanceExtensions(&count);
                if (!sdl_exts) {
                    throw std::runtime_error(std::format("Failed to query for mandatory Vulkan instance extensions: {}", SDL_GetError()));
                }

                for (int i = 0; i < count; i++) {
                    if (!supports_instance_extension(sdl_exts[i])) {
                        throw std::runtime_error(
                            std::format("System doesn't support the required Vulkan instance extension {}", sdl_exts[i]));
                    }
                    m_enabled_instance_exts.push_back(sdl_exts[i]);
                }

                if (const char* error; !supports_instance_extensions(
                        { VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME, VK_KHR_EXTERNAL_FENCE_CAPABILITIES_EXTENSION_NAME,
                            VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME },
                        &error)) {
                    throw std::runtime_error(std::format("System doesn't support the required Vulkan instance extension {}", error));
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
                .pApplicationName = GAME_NAME,
                .applicationVersion = VK_MAKE_VERSION(GAME_VERSION_MAJOR, GAME_VERSION_MINOR, GAME_VERSION_PATCH),
                .pEngineName = ENGINE_NAME,
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
                .ppEnabledExtensionNames = m_enabled_instance_exts.data() };

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
                    if (queue_family.queueCount > 0 && queue_family.queueFlags & VK_QUEUE_COMPUTE_BIT && !(queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
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
                    if (queue_family.queueCount > 0 && queue_family.queueFlags & VK_QUEUE_TRANSFER_BIT && !(queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) && !(queue_family.queueFlags & VK_QUEUE_COMPUTE_BIT)) {
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

                m_physical_device_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
                m_physical_device_features11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
                m_physical_device_features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
                m_physical_device_syncronization2_features_khr.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
                m_physical_device_dynamic_rendering_features_khr.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;

                m_physical_device_features.pNext = &m_physical_device_features11;
                m_physical_device_features11.pNext = &m_physical_device_features12;
                m_physical_device_features12.pNext = &m_physical_device_syncronization2_features_khr;
                m_physical_device_syncronization2_features_khr.pNext   = &m_physical_device_dynamic_rendering_features_khr;
                m_physical_device_dynamic_rendering_features_khr.pNext = nullptr;

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

                if (const char* error; !supports_device_extensions({ 
                    VK_KHR_SWAPCHAIN_EXTENSION_NAME, 
                    VK_EXT_MEMORY_BUDGET_EXTENSION_NAME,
                    VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME, 
                    VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME })) {
                    return false;
                }

#if defined(SDL_PLATFORM_WIN32)
                if (const char* error; 
                    !supports_device_extensions({
                        VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME, 
                        VK_KHR_EXTERNAL_FENCE_WIN32_EXTENSION_NAME,
                        VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME})) {
                    return false;
                }
#elif defined(SDL_PLATFORM_LINUX)
                // TODO
#endif
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
           
             // std::map<u32, VkPhysicalDevice, std::greater<>> m_device_ranking;

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
                score += find_dedicated_cmp_queue(physical_device) != VK_QUEUE_FAMILY_IGNORED ? 500 : 0;
                score += find_dedicated_trs_queue(physical_device) != VK_QUEUE_FAMILY_IGNORED ? 1000 : 0;
                physical_device_ranker.emplace(score, physical_device);
            }
            if (physical_device_ranker.empty()) {
                throw std::runtime_error("Failed to find any suitable Vulkan physical devices");
            }

            m_physical_device = physical_device_ranker.begin()->second;
            check_device_properties(m_physical_device);

            if (m_has_memory_budget) {
                m_enabled_device_exts.push_back(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME);
            }

            m_queue_indices[(u32) CommandQueue::kGeneral] = find_gfx_queue(m_physical_device);
            m_queue_indices[(u32)CommandQueue::kCopy] = find_dedicated_trs_queue(m_physical_device);
            m_queue_indices[(u32)CommandQueue::kCompute] = find_dedicated_cmp_queue(m_physical_device);

            // Separate transfer/compute families if no dedicated are present
            {
                u32 queue_family_count = 0;
                vkGetPhysicalDeviceQueueFamilyProperties(m_physical_device, &queue_family_count, nullptr);
                std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
                vkGetPhysicalDeviceQueueFamilyProperties(m_physical_device, &queue_family_count, queue_families.data());

                for (u32 i = 0; i < queue_family_count; ++i) {
                    auto& queueFamily = queue_families[i];
                    if (queueFamily.queueCount > 0 &&
                        queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT &&
                        i != m_queue_indices[(u32) CommandQueue::kGeneral] &&
                        m_queue_indices[(u32)CommandQueue::kCopy] == VK_QUEUE_FAMILY_IGNORED) {
                        m_queue_indices[(u32)CommandQueue::kCopy] = i;
                        RHI_WARN("Using non-dedicated Vulkan transfer queue");
                        continue;
                    }
                    if (queueFamily.queueCount > 0 &&
                        queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT &&
                        i != m_queue_indices[(u32) CommandQueue::kGeneral] &&
                        m_queue_indices[(u32)CommandQueue::kCompute] == VK_QUEUE_FAMILY_IGNORED) {
                        m_queue_indices[(u32)CommandQueue::kCompute] = i;
                        RHI_WARN("Using non-dedicated Vulkan compute queue");
                        continue;
                    }
                }
            }
            
            // Just choose one queue if no separate ones are available
            {
                if(m_queue_indices[(u32)CommandQueue::kCopy] == VK_QUEUE_FAMILY_IGNORED) {
                    m_queue_indices[(u32)CommandQueue::kCopy] = m_queue_indices[(u32) CommandQueue::kGeneral];
                    RHI_WARN("Using non-separated Vulkan transfer queue");
                }
                if(m_queue_indices[(u32)CommandQueue::kCompute] == VK_QUEUE_FAMILY_IGNORED) {
                    m_queue_indices[(u32)CommandQueue::kCompute] = m_queue_indices[(u32) CommandQueue::kGeneral];
                    RHI_WARN("Using non-separated Vulkan compute queue");
                }
            }
            
            std::unordered_set<u32> unique_families = {
                m_queue_indices[(u32)CommandQueue::kGeneral],
                m_queue_indices[(u32)CommandQueue::kCompute], 
                m_queue_indices[(u32)CommandQueue::kCopy] 
            };
            
            m_single_queue = unique_families.size() == 1;
            
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
            
            /*
            m_maxSamplersDescriptorCount
                = std::min(MAX_SAMPLER_DESCRIPTORS, m_physicalDeviceProperties.properties.limits.maxDescriptorSetSamplers);

            m_maxSampledImagesDescriptorCount
                = std::min(MAX_SAMPLED_IMAGE_DESCRIPTORS, m_physicalDeviceVulkan12Properties.maxDescriptorSetUpdateAfterBindSampledImages);

            m_maxStorageImagesDescriptorCount
                = std::min(MAX_STORAGE_IMAGE_DESCRIPTORS, m_physicalDeviceVulkan12Properties.maxDescriptorSetUpdateAfterBindStorageImages);

            m_maxStorageBuffersDescriptorCount
                = std::min(MAX_STORAGE_BUFFER_DESCRIPTORS, m_physicalDeviceProperties.properties.limits.maxDescriptorSetStorageBuffers);

            m_maxUniformBuffersDescriptorCount
                = std::min(MAX_STORAGE_BUFFER_DESCRIPTORS, m_physicalDeviceProperties.properties.limits.maxDescriptorSetUniformBuffers);
            */
            RHI_INFO("Vulkan Driver {0} created successfully", m_physical_device_props.properties.deviceName);
            RHI_INFO("ReBar/UMA: {0}, VRAM Size: {1}, VK_EXT_MEMORY_BUDGET: {2}", m_rebar, m_memory_size, m_has_memory_budget);
            /*
            spdlog::debug("\nBindless resource limits:"
                          "\n\t Samplers: {0}"
                          "\n\t Sampled images:  {1}"
                          "\n\t Storage images:  {2}"
                          "\n\t Storage buffers: {3}"
                          "\n\t Uniform buffers: {4}",
                m_maxSamplersDescriptorCount, m_maxSampledImagesDescriptorCount, m_maxStorageImagesDescriptorCount,
                m_maxStorageBuffersDescriptorCount, m_maxUniformBuffersDescriptorCount);
                */
        }
    }
    
    VulkanDevice::~VulkanDevice() 
    { 
        vkDestroyDevice(m_device, nullptr);

        if (m_debug) {
           vkDestroyDebugUtilsMessengerEXT(m_instance, m_debug_messenger, nullptr);
        }
        vkDestroyInstance(m_instance, nullptr);
    }

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

    bool VulkanDevice::supports_instance_extensions(std::initializer_list<const char*> extension_names, const char** unsupported_ext) const {
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
    
    bool VulkanDevice::supports_device_extensions(std::initializer_list<const char*> extension_names, const char** unsupported_ext) const 
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
}
