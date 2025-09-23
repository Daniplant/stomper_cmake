#pragma once
#include "../sys_rhi.hpp"
#include "engine/common.hpp"
#include "engine/rhi/rhi.hpp"

#include <mutex>
#include <thread>
#include <atomic>

#include <array>
#include <vector>
#include <unordered_map>

#include <Metal/Metal.hpp>


namespace core::rhi
{
    struct MetalFence final : public Fence
    {
        MetalFence(MTL::Device* device)
        {
            refcount.store(0);
            completed.store(false);
            event = device->newEvent();
        }
    
        ~MetalFence() override
        {
            event->release();
        }
        
        MTL::Event* event;
        std::atomic_bool completed;
        std::atomic_int32_t refcount;
    };

    struct MetalCommandBuffer final : public CommandBuffer
    {
        MetalCommandBuffer()
        {
            handle = nullptr;
            fence = nullptr;
            autorelease = true;
        }

        ~MetalCommandBuffer() override
        {
            handle->release();
        }

        void reset()
        {
            fence = nullptr;
            autorelease = true;
            handle->release();
            handle = nullptr;
        }
        
        bool autorelease;
        MetalFence* fence;
        MTL::CommandBuffer* handle;
    };

    class MetalDevice final : public Device {
        MetalDevice(bool debug);
        ~MetalDevice() override;

        bool supports_advanced_sync() override { return true; }
        
        bool create_swapchain(SDL_Window* window) override {return false;}
        void destroy_swapchain(SDL_Window* window) override {};

        CommandBuffer* begin_commandbuffer(CommandQueue queue) override;
        void end_commandbuffer(CommandBuffer* cmd) override;
        
        bool submit_commandbuffers(std::span<CommandBuffer*> cmds) override;
        
        std::string get_name() const override;
        
    private:
        MetalFence* fetch_fence();
        MetalCommandBuffer* fetch_commandbuffer();
        
    private:
        MTL::Device* m_device;
        MTL::CommandQueue* m_command_queue;
        std::mutex m_fence_pool_mtx;
        std::mutex m_commandbuffer_submit_mtx;
        std::mutex m_commandbuffer_acquire_mtx;
        
        std::vector<MetalFence*> m_fence_pool;
        std::vector<MetalCommandBuffer*> m_commandbuffers_pool;
        std::vector<MetalCommandBuffer*> m_submitted_commandbuffers;
    };
}

// This was the Metal4 version I was writing first.
// It's not really worth it, I want x86_64 support still
/*
namespace core::rhi
{
    struct MetalCommandAllocator
    {
        MetalCommandAllocator()
        {
            handle = nullptr;
        }

        ~MetalCommandAllocator()
        {
            handle->reset();
            handle->release();
        }

        void reset()
        {
            handle->reset();
        }

        MTL4::CommandAllocator* handle;
    };

    struct MetalCommandBuffer final : public CommandBuffer
    {
        MetalCommandBuffer()
        {
            handle    = nullptr;
            allocator = nullptr;
        }

        ~MetalCommandBuffer() override
        {
            handle->release();
        }

        void reset()
        {
            allocator = nullptr;
        }

        CommandQueue queue;
        MTL4::CommandBuffer* handle;
        MetalCommandAllocator* allocator;
    };

    struct MetalFence final : public Fence {
    public:
        MetalFence() = default;
        ~MetalFence() override = default;
        
        MTL::SharedEvent* handle;
    };

    class MetalDevice final : public Device
    {
    public:
        MetalDevice(bool debug);
        ~MetalDevice() override;

        bool supports_advanced_sync() override { return true; }
        
        bool create_swapchain(SDL_Window* window) override;
        void destroy_swapchain(SDL_Window* window) override;

        CommandBuffer* begin_commandbuffer(CommandQueue queue) override;
        void end_commandbuffer(CommandBuffer* cmd) override;
        
        bool submit_commandbuffers(std::span<CommandBuffer*> cmds) override;
        
        std::string get_name() const override;
    
    private:
        bool allocate_command_allocator(std::thread::id thread_id);
        bool allocate_commandbuffer();
        
        MetalCommandAllocator* fetch_command_allocator(std::thread::id thread_id);
        MetalCommandBuffer* fetch_commandbuffer();
        
    private:
        MTL::Device* m_device;
        std::mutex m_command_allocator_mtx, m_commandbuffer_acquire_mtx, m_commandbuffer_submit_mtx;
        
        std::vector<MetalCommandBuffer*> m_free_commandbuffers_pool, m_submitted_commandbuffers;
        std::array<MTL4::CommandQueue*, (u32) CommandQueue::kMax> m_command_queues;
        std::unordered_map<std::thread::id, std::vector<MetalCommandAllocator*>> m_command_allocator_pool;
    };
} // namespace core::rhi

*/
