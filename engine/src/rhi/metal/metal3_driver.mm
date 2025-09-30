#include "metal_driver_base.hpp"

#import <Metal/Metal.h>
#import <Foundation/Foundation.h>

#include <SDL3/SDL_metal.h>
#include <SDL3/SDL_stdinc.h>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/bundled/format.h>

#define INITIAL_HEAP_SIZE 256 * 1024 * 1024

namespace core::rhi
{
    struct MetalFence final : public Fence
    {
    public:
        MetalFence() = default;
        ~MetalFence() override = default;
        
    };

    struct MetalCommandBuffer final : public CommandBuffer
    {
    public:
        MetalCommandBuffer() = default;
        ~MetalCommandBuffer() override = default;
    };

    class MetalDevice final : public Device
    {
    public:
        MetalDevice(bool debug) : m_debug(debug)
        {
            @autoreleasepool
            {
                if(m_debug) {
                    SDL_setenv_unsafe("MTL_DEBUG_LAYER", "1", 0);
                }
                
                m_device = MTLCreateSystemDefaultDevice();
                if(!m_device) {
                    throw std::runtime_error("Failed to create Metal device");
                }
                if(![m_device supportsFamily:MTLGPUFamilyMetal3]){
                    throw std::runtime_error("Failed to create Metal RHI, you need a Metal 3 capable device to run this application");
                }
                
                m_queue = [m_device newCommandQueue];
                if(!m_queue) {
                    throw std::runtime_error("Failed to create Metal queue");
                }
                
                MTLHeapDescriptor* heap_desc = [[MTLHeapDescriptor alloc] init];
                heap_desc.size = INITIAL_HEAP_SIZE;
                heap_desc.type = MTLHeapTypeAutomatic;
                
                m_rendertargets_heap = [m_device newHeapWithDescriptor:heap_desc];
                if(!m_rendertargets_heap) {
                    throw std::runtime_error("Failed to create render targets heap");
                }
                if(m_debug) {
                    m_rendertargets_heap.label = @"Render targets heap";
                }
                
                heap_desc.type = MTLHeapTypePlacement;
                
                m_textures_heap = [m_device newHeapWithDescriptor:heap_desc];
                if(!m_textures_heap) {
                    throw std::runtime_error("Failed to create textures heap");
                }
                if(m_debug) {
                    m_textures_heap.label = @"Textures heap";
                }
                
                m_buffers_heap = [m_device newHeapWithDescriptor:heap_desc];
                if(!m_buffers_heap) {
                    throw std::runtime_error("Failed to create buffers heap");
                }
                if(m_debug) {
                    m_buffers_heap.label = @"Buffers heap";
                }
                
                m_name = std::string([[m_device name] UTF8String]);
            }
        }
        
        ~MetalDevice() override
        {
            
        }
        
        bool supports_advanced_sync() override
        {
            return true;
        }
        
        bool create_swapchain(SDL_Window* window) override
        {
            throw std::runtime_error("Not yet implemented");
        }
        
        void destroy_swapchain(SDL_Window* window) override
        {
            throw std::runtime_error("Not yet implemented");
        }

        CommandBuffer* begin_commandbuffer(CommandQueue queue) override
        {
            throw std::runtime_error("Not yet implemented");
        }
        
        void end_commandbuffer(CommandBuffer* cmd) override
        {
            throw std::runtime_error("Not yet implemented");
        }
            
        bool submit_commandbuffers(std::span<CommandBuffer*> cmds) override
        {
            throw std::runtime_error("Not yet implemented");
        }
        
        std::string get_name() const override
        {
            return m_name;
        }
        
    private:
        bool m_debug;
        std::string m_name;
        
        id<MTLDevice> m_device;
        id<MTLCommandQueue> m_queue;
        
        id<MTLHeap> m_buffers_heap;
        id<MTLHeap> m_textures_heap;
        id<MTLHeap> m_rendertargets_heap;
        
        std::mutex m_fence_pool_mtx;
        std::mutex m_commandbuffer_submit_mtx;
        std::mutex m_commandbuffer_acquire_mtx;
        
        std::vector<MetalFence*> m_fence_pool;
        std::vector<MetalCommandBuffer*> m_commandbuffers_pool;
        std::vector<MetalCommandBuffer*> m_submitted_commandbuffers;
    };
    
    std::unique_ptr<Device> make_metal3_device(bool debug)
    {
        return std::make_unique<MetalDevice>(debug);
    }
}
