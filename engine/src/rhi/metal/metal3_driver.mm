#include "metal_driver_base.hpp"

#import <Metal/Metal.h>
#import <Foundation/Foundation.h>
#import <QuartzCore/CoreAnimation.h>

#include <SDL3/SDL_metal.h>
#include <SDL3/SDL_stdinc.h>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/bundled/format.h>

#define INITIAL_HEAP_SIZE 256 * 1024 * 1024

namespace core::rhi
{
    struct MetalFence;

    struct MetalTexture : TextureBase {
        id<MTLTexture> handle;
    };

    struct MetalSwapchain {
        SDL_Window* window;
        SDL_MetalView view;
        CAMetalLayer* layer;
        id<CAMetalDrawable> drawable;
        
        MetalTexture texture;
        
        u64 frame_count;
        std::array<MetalFence*, MAX_SWAPCHAIN_FRAMES> frame_fences;
    };

    struct MetalFence final : public Fence
    {
        MetalFence() = default;
        ~MetalFence() override = default;
        
        bool wait() override
        {
            while(!completed.load()) {
                // spiiiiiiiiin
            }
            return true;
        }
        
        bool is_signaled() override
        {
            return completed.load();
        }
        
        std::atomic_bool completed;
    };

    struct MetalEvent {
    
    };

    struct MetalCommandBuffer final : public CommandBuffer
    {
        MetalCommandBuffer() = default;
        ~MetalCommandBuffer() override = default;
        
        id<MTLCommandBuffer> handle;
        
        std::vector<MetalSwapchain*> bound_swapchains;
        
        std::vector<MetalEvent*> wait_events;
        std::vector<MetalEvent*> signal_events;
        
        MetalFence* fence;
        bool autorelease_fence;
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
                
                RHI_INFO("Metal 3 RHI created successfully");
                RHI_INFO("Using GPU device {}", get_name());
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
            // There's only one queue, no need for this parameter
            std::ignore = queue;
            
            @autoreleasepool
            {
                MetalCommandBuffer* cmd = fetch_commandbuffer();
                if(!cmd){
                    RHI_ERROR("Failed to acquire command buffer");
                    return nullptr;
                }
                
                cmd->handle = [m_queue commandBuffer];
                cmd->autorelease_fence = true;
                
                return cmd;
            }
        }
        
        // Function does nothing in Metal 3
        void end_commandbuffer(CommandBuffer* cmd) override
        {
            std::ignore = cmd;
        }
            
        bool submit_commandbuffer(CommandBuffer* cmd) override
        {
            @autoreleasepool
            {
                auto metal_cmd = static_cast<MetalCommandBuffer*>(cmd);
                    
                metal_cmd->fence = fetch_fence();
                if(!metal_cmd->fence){
                    RHI_ERROR("Failed to acquire fence");
                    return false;
                }
                    
                [metal_cmd->handle addCompletedHandler:^(id<MTLCommandBuffer>) {
                    metal_cmd->fence->completed.store(true);
                }];
                    
                [metal_cmd->handle commit];
                metal_cmd->handle = nil;
                    
                m_cmdbuffers_submit_mtx.lock();
                m_submitted_cmdbuffers.push_back(metal_cmd);
                m_cmdbuffers_submit_mtx.unlock();
            }
        
            // eventually clean up resources
            return true;
        }
        
        Fence* submit_commandbuffer_fenced(CommandBuffer* cmd) override
        {
            if(!submit_commandbuffer(cmd)){
                return nullptr;
            }
            
            auto metal_cmd = static_cast<MetalCommandBuffer*>(cmd);
            metal_cmd->autorelease_fence = false;
            return metal_cmd->fence;
        }
        
        std::string get_name() const override
        {
            return m_name;
        }
        
    private:
        MetalCommandBuffer* fetch_commandbuffer()
        {
            std::lock_guard lock(m_cmdbuffers_acquire_mtx);
            
            if(m_cmdbuffers_pool.empty()) {
                m_cmdbuffers_pool.emplace_back(new MetalCommandBuffer());
            }
            
            MetalCommandBuffer* cmd = m_cmdbuffers_pool.back();
            m_cmdbuffers_pool.pop_back();
            return cmd;
        }
        
        MetalFence* fetch_fence() {
            std::lock_guard lock(m_fence_pool_mtx);
            
            if(m_fences_pool.empty()) {
                m_fences_pool.emplace_back(new MetalFence());
            }
            
            MetalFence* fence = m_fences_pool.back();
            m_fences_pool.pop_back();
            return fence;
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
        std::mutex m_cmdbuffers_submit_mtx;
        std::mutex m_cmdbuffers_acquire_mtx;
        
        std::vector<MetalFence*> m_fences_pool;
        std::vector<MetalCommandBuffer*> m_cmdbuffers_pool;
        std::vector<MetalCommandBuffer*> m_submitted_cmdbuffers;
    };
    
    std::unique_ptr<Device> make_metal3_device(bool debug)
    {
        return std::make_unique<MetalDevice>(debug);
    }
}
