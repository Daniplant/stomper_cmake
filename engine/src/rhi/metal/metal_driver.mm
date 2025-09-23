#define NS_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION

#include "metal_driver.hpp"

#include <memory>
#include <spdlog/spdlog.h>
#include <SDL3/SDL_stdinc.h>
#include <Foundation/Foundation.hpp>


namespace core::rhi
{
    MetalDevice::MetalDevice(bool debug)
    {
        if(debug) {
            SDL_setenv_unsafe("MTL_DEBUG_LAYER", "1", 0);
        }
        
        m_device = MTL::CreateSystemDefaultDevice();
        if(!m_device || !m_device->supportsFamily(MTL::GPUFamilyMetal3)) {
            throw std::runtime_error("Failed to create Metal device");
        }
        
        m_command_queue = m_device->newCommandQueue();
        if(!m_command_queue){
            throw std::runtime_error("Failed to create Metal command queue");
        }
    }

    CommandBuffer* MetalDevice::begin_commandbuffer(CommandQueue queue)
    {
        std::ignore = queue;
        
        MetalCommandBuffer* cmd = fetch_commandbuffer();
        if(!cmd){
            RHI_ERROR("Failed to acquire command buffer");
            return nullptr;
        }
        
        cmd->fence = fetch_fence();
        if(!cmd->fence){
            RHI_ERROR("Failed to acquire command buffer fence");
            return nullptr;
        }
        cmd->handle = m_command_queue->commandBufferWithUnretainedReferences();
        cmd->autorelease = true;
        return cmd;
    }

    void MetalDevice::end_commandbuffer(CommandBuffer* cmd)
    {
        auto metal_cmd = static_cast<MetalCommandBuffer*>(cmd);
        metal_cmd->handle->addCompletedHandler(^(MTL::CommandBuffer *) {
            metal_cmd->fence->completed.store(true);
        });
    }
    
    MetalFence* MetalDevice::fetch_fence()
    {
        std::lock_guard lock(m_fence_pool_mtx);
        
        if(m_fence_pool.empty()) {
            m_fence_pool.push_back(new MetalFence(m_device));
        }
        
        auto* fence = m_fence_pool.back();
        m_fence_pool.pop_back();
        return fence;
    }

    MetalCommandBuffer* MetalDevice::fetch_commandbuffer()
    {
        std::lock_guard lock(m_commandbuffer_acquire_mtx);
        
        if(m_commandbuffers_pool.empty()) {
            m_commandbuffers_pool.emplace_back(new MetalCommandBuffer());
        }
        
        auto* cmd = m_commandbuffers_pool.back();
        m_commandbuffers_pool.pop_back();
        return cmd;
    }

}

/*
namespace core::rhi
{
    MetalDevice::MetalDevice(bool debug)
    {
        if(debug) {
            SDL_setenv_unsafe("MTL_DEBUG_LAYER", "1", 0);
        }
        
        m_device = MTL::CreateSystemDefaultDevice();
        if(!m_device || !m_device->supportsFamily(MTL::GPUFamilyMetal4)) {
            throw std::runtime_error("Failed to create Metal device");
        }

        for(auto& queue: m_command_queues) {
            queue = m_device->newMTL4CommandQueue();
            if(!queue) {
                throw std::runtime_error("Failed to create Metal command queue");
            }
        }
        
        if(!allocate_command_allocator(std::this_thread::get_id())) {
            throw std::runtime_error("Failed to create Metal command allocator");
        }
        
        if(!allocate_commandbuffer()) {
            throw std::runtime_error("Failed to create Metal command buffer");
        }
        
        RHI_INFO("Metal RHI created successfuly");
        RHI_INFO("Using GPU device {}", get_name());
    }
    
    MetalDevice::~MetalDevice()
    {
        for(auto& queue: m_command_queues) {
            queue->release();
        }
        for(auto& cmd : m_free_commandbuffers_pool) {
            delete cmd;
        }
        for(auto&[thread_id, value] : m_command_allocator_pool) {
            for(auto& allocator : value) {
                delete allocator;
            }
        }
        m_device->release();
        RHI_INFO("Metal RHI destroyed successfuly");
    }
    
    bool MetalDevice::create_swapchain(SDL_Window *window)
    {
        throw std::logic_error("Not yet implemented");
    }
    
    void MetalDevice::destroy_swapchain(SDL_Window *window)
    {
        throw std::logic_error("Not yet implemented");
    }
    
    CommandBuffer* MetalDevice::begin_commandbuffer(CommandQueue queue)
    {
        // NOTE: This way of handling command allocators (one per command buffer) is
        // fenomenally stupid and memory-wasteful, but as a first implementation it'll do.
        // In an indeal world we would assign as many command buffers per frame,
        // per thread to the same allocator and free on completition
        
        MetalCommandAllocator* allocator = fetch_command_allocator(std::this_thread::get_id());
        if(!allocator){
            RHI_ERROR("Failed to fetch command buffer allocator");
            return nullptr;
        }
        
        MetalCommandBuffer* commandbuffer = fetch_commandbuffer();
        if(!commandbuffer){
            RHI_ERROR("Failed to fetch command buffer");
            return nullptr;
        }
        
        commandbuffer->queue = queue;
        commandbuffer->allocator = allocator;
        commandbuffer->handle->beginCommandBuffer(allocator->handle);
        return commandbuffer;
    }

    void MetalDevice::end_commandbuffer(CommandBuffer* cmd)
    {
        static_cast<MetalCommandBuffer*>(cmd)->handle->endCommandBuffer();
    }
    
    bool MetalDevice::submit_commandbuffers(std::span<CommandBuffer*> cmds)
    {
        std::vector<MTL4::CommandBuffer*> graphics_cmds;
        std::vector<MTL4::CommandBuffer*> compute_cmds;
        std::vector<MTL4::CommandBuffer*> copy_cmds;
        
        for(CommandBuffer* cmd : cmds) {
            auto* metal_cmd = static_cast<MetalCommandBuffer*>(cmd);
            
            switch (metal_cmd->queue) {
                case CommandQueue::kGeneral:
                    graphics_cmds.push_back(metal_cmd->handle);
                    break;
                case CommandQueue::kCompute:
                    compute_cmds.push_back(metal_cmd->handle);
                    break;
                case CommandQueue::kCopy:
                    copy_cmds.push_back(metal_cmd->handle);
                    break;
                default:
                    break;
            }
            metal_cmd->handle->
            m_commandbuffer_submit_mtx.lock();
            m_submitted_commandbuffers.push_back(metal_cmd);
            m_commandbuffer_submit_mtx.unlock();
        }
    
        if(!graphics_cmds.empty()){
            m_command_queues[(u32) CommandQueue::kGeneral]->commit(graphics_cmds.data(), graphics_cmds.size());
        }
        if(!compute_cmds.empty()){
            m_command_queues[(u32) CommandQueue::kCompute]->commit(compute_cmds.data(), compute_cmds.size());
        }
        if(!copy_cmds.empty()){
            m_command_queues[(u32) CommandQueue::kCopy]->commit(copy_cmds.data(), copy_cmds.size());
        }
        return true;
    }

    std::string MetalDevice::get_name() const
    {
        return m_device->name()->utf8String();
    }

    bool MetalDevice::allocate_command_allocator(std::thread::id thread_id)
    {
        auto cmd_allocator = std::make_unique<MetalCommandAllocator>();
        cmd_allocator->handle = m_device->newCommandAllocator();
        if(!cmd_allocator->handle) {
            return false;
        }
        m_command_allocator_pool[thread_id].push_back(cmd_allocator.release());
        return true;
    }
    
    bool MetalDevice::allocate_commandbuffer()
    {
        auto cmd_buffer = std::make_unique<MetalCommandBuffer>();
        cmd_buffer->handle = m_device->newCommandBuffer();
        if(!cmd_buffer->handle) {
            return false;
        }
        m_free_commandbuffers_pool.push_back(cmd_buffer.release());
        return true;
    }

    MetalCommandAllocator* MetalDevice::fetch_command_allocator(std::thread::id thread_id)
    {
        std::lock_guard lock(m_command_allocator_mtx);

        auto& pool = m_command_allocator_pool[thread_id];
        if(pool.empty()){
            if(!allocate_command_allocator(thread_id)){
                return nullptr;
            }
        }
        
        auto allocator = pool.back();
        pool.pop_back();
        return allocator;
    }

    MetalCommandBuffer* MetalDevice::fetch_commandbuffer()
    {
        std::lock_guard lock(m_commandbuffer_acquire_mtx);
        
        if(m_free_commandbuffers_pool.empty()){
            if(!allocate_commandbuffer()){
                return nullptr;
            }
        }
        
        auto commandbuffer = m_free_commandbuffers_pool.back();
        m_free_commandbuffers_pool.pop_back();
        return commandbuffer;
    }
}

*/
