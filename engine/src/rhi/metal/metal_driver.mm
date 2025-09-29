#include "metal_driver.hpp"

#import <Metal/Metal.h>
#import <Foundation/Foundation.h>

#include <spdlog/spdlog.h>
#include <SDL3/SDL_stdinc.h>

#define INITIAL_HEAP_SIZE 256 * 1024 * 1024

namespace core::rhi
{
    
    struct MetalFenceImpl final : public MetalFence<MetalFenceImpl>
    {
    public:
        MetalFenceImpl() = default;
        ~MetalFenceImpl() override = default;
        
    };

    struct MetalCommandBufferImpl final : public MetalCommandBuffer<MetalCommandBufferImpl>
    {
    public:
        MetalCommandBufferImpl() = default;
        ~MetalCommandBufferImpl() override = default;
    };

    class MetalDeviceImpl final : public MetalDevice<MetalDeviceImpl>
    {
    public:
        
        MetalDeviceImpl(bool debug)
        {
            @autoreleasepool
            {
                if(debug) {
                    SDL_setenv_unsafe("MTL_DEBUG_LAYER", "1", 0);
                }
                
                m_device = MTLCreateSystemDefaultDevice();
                if(!m_device) {
                    throw std::runtime_error("Failed to create Metal device");
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
                if(debug) {
                    m_rendertargets_heap.label = @"Render targets heap";
                }
                
                heap_desc.type = MTLHeapTypePlacement;
                
                m_textures_heap = [m_device newHeapWithDescriptor:heap_desc];
                if(!m_textures_heap) {
                    throw std::runtime_error("Failed to create textures heap");
                }
                if(debug) {
                    m_textures_heap.label = @"Textures heap";
                }
                
                m_buffers_heap = [m_device newHeapWithDescriptor:heap_desc];
                if(!m_buffers_heap) {
                    throw std::runtime_error("Failed to create buffers heap");
                }
                if(debug) {
                    m_buffers_heap.label = @"Buffers heap";
                }
                
                
            }
        }
        
        ~MetalDeviceImpl() override
        {
            
        }
        
        bool supports_advanced_sync_impl()
        {
            return true;
        }
        
        bool create_swapchain_impl(SDL_Window* window)
        {
            throw std::runtime_error("Not yet implemented");
        }
        
        void destroy_swapchain_impl(SDL_Window* window)
        {
            throw std::runtime_error("Not yet implemented");
        }

        CommandBuffer* begin_commandbuffer_impl(CommandQueue queue)
        {
            throw std::runtime_error("Not yet implemented");
        }
        
        void end_commandbuffer_impl(CommandBuffer* cmd)
        {
            throw std::runtime_error("Not yet implemented");
        }
            
        bool submit_commandbuffers_impl(std::span<CommandBuffer*> cmds)
        {
            throw std::runtime_error("Not yet implemented");
        }
        
        std::string get_name_impl() const
        {
            return std::string([[m_device name] UTF8String]);
        }
        
    private:
        id<MTLDevice> m_device;
        id<MTLCommandQueue> m_queue;
        
        id<MTLHeap> m_buffers_heap;
        id<MTLHeap> m_textures_heap;
        id<MTLHeap> m_rendertargets_heap;
        
        std::mutex m_fence_pool_mtx;
        std::mutex m_commandbuffer_submit_mtx;
        std::mutex m_commandbuffer_acquire_mtx;
        
        std::vector<MetalFenceImpl*> m_fence_pool;
        std::vector<MetalCommandBufferImpl*> m_commandbuffers_pool;
        std::vector<MetalCommandBufferImpl*> m_submitted_commandbuffers;
    };

    std::unique_ptr<Device> make_metal_device(bool debug)
    {
        return std::make_unique<MetalDeviceImpl>(debug);
    }
}
