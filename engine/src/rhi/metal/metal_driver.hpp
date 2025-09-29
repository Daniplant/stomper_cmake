#pragma once
#include "../sys_rhi.hpp"
#include "engine/common.hpp"
#include "engine/rhi/rhi.hpp"

#include <mutex>
#include <memory>

namespace core::rhi
{
    template <typename Impl>
    class MetalFence : public Fence
    {
    protected:
        MetalFence() = default;
        ~MetalFence() override = default;
    public:
        
    };

    template <typename Impl>
    class MetalCommandBuffer : public CommandBuffer
    {
    protected:
        MetalCommandBuffer() = default;
        ~MetalCommandBuffer() override = default;
    public:
        
    };

    template <typename Impl>
    class MetalDevice : public Device
    {
    protected:
        MetalDevice() = default;
        ~MetalDevice() override = default;
        
    public:
        bool supports_advanced_sync() override
        {
            return static_cast<Impl*>(this)->supports_advanced_sync_impl();
        }
            
        bool create_swapchain(SDL_Window* window) override
        {
            return static_cast<Impl*>(this)->create_swapchain_impl(window);
        }
        
        void destroy_swapchain(SDL_Window* window) override
        {
            static_cast<Impl*>(this)->destroy_swapchain_impl(window);
        }

        CommandBuffer* begin_commandbuffer(CommandQueue queue) override
        {
            return static_cast<Impl*>(this)->begin_commandbuffer_impl(queue);
        }
        
        void end_commandbuffer(CommandBuffer* cmd) override
        {
            static_cast<Impl*>(this)->end_commandbuffer_impl(cmd);
        }
            
        bool submit_commandbuffers(std::span<CommandBuffer*> cmds) override
        {
            return static_cast<Impl*>(this)->submit_commandbuffers_impl(cmds);
        }
            
        std::string get_name() const override
        {
            return static_cast<const Impl*>(this)->get_name_impl();
        }
    };

    std::unique_ptr<Device> make_metal_device(bool debug);
}
