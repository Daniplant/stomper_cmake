#pragma once
#include "engine/common.hpp"

#include <span>
#include <string>
#include <SDL3/SDL_video.h>

namespace core::rhi
{
    CORE_MAKE_HANDLE(Buffer)
    CORE_MAKE_HANDLE(Texture)

    /**
     * Specifies the swapchain present mode
     */
    enum class SwapchainPresentMode : u32
    {
        kVsync,
        kImmediate,
        kMailbox,
        kMax
    };

    /**
     * Specifies the swapchain colospace
     */
    enum class SwapchainColorspace : u32
    {
        kSdr,
        kSdrLinear,
        kHdrExtendedLinear,
        kHdr10St2048,
    };

    /**
     * Specifies the sample count of a texture
     */
    enum class SampleCount : u32
    {
        k1,  /** No multisampling */
        k2,  /** MSAA 2 */
        k4,  /** MSAA 4 */
        k8,  /** MSAA 8 */
        k16, /** MSAA 16 because why the FUCK NOT */
        kMax
    };

    /**
     * Specifies how a texture should be used by the rhi
     *
     * Note that StorageRead | StorageWrite is NOT equal to StorageReadWrite.
     * StorageRead | StorageWrite means that:
     *      Shader A can read from the texture
     *      Shader B can write to the texture (or vice versa)
     * You CANNOT write and read to the same texture in the same shader
     * For that, use StorageReadWrite (this flag is only supported in compute
     * shaders and select texture formats)
     */
    enum class TextureUsageFlags : u32
    {
        kSampler            = u32(1) << 0, /** Enables texture sampling in shaders */
        kColorTarget        = u32(1) << 1, /**  Enables texture for color target output */
        kDepthStencilTarget = u32(1) << 2, /**  Enables texture for depth and/or stencil target output */
        kStorageRead        = u32(1) << 3, /**  Enables texture for read-only storage ops */
        kStorageWrite       = u32(1) << 4, /**  Enables texture for write-only storage ops */
        kStorageReadWrite   = u32(1) << 5, /**  Enables texture simultaneous read-write storage ops */
        kMax
    };
    CORE_MAKE_SCOPED_ENUM_BITOPS(TextureUsageFlags)

    enum class TextureType : u32
    {
        k2d,        /** 2D texture */
        k2dArray,   /** Array of 2D textures */
        k2dMs,      /** Multisampled 2D texture */
        k2dMsArray, /** Array of multisampled 2D textures */
        k3d,        /** 3D texture */
        kCube,      /** Cubemap texture */
        kCubeArray, /** Array of cubemap textures */
        kMax
    };

    enum class TextureFormat : u32
    {
    };

    enum class CommandQueue : u32
    {
        kGeneral, /** General queue, usable for graphics, compute and copy operations */
        kCompute, /** Async compute queue, usable for compute and copy operations */
        kCopy,    /** Copy queue, usable only for copy operations */
        kMax
    };

    struct TextureDescriptor
    {
        u32 width;
        u32 height;
        u32 depth_or_layers;
        u32 mip_levels;
        TextureType type;
        TextureFormat format;
        TextureUsageFlags usage;
        SampleCount samples;
    };
    
    class Fence
    {
        CORE_MAKE_INTERFACE_PROTECTED(Fence)
    public:
        
    };

    class Renderpass {
        CORE_MAKE_INTERFACE_PROTECTED(Renderpass)
    public:
    
    };

    class CommandBuffer
    {
        CORE_MAKE_INTERFACE_PROTECTED(CommandBuffer)
        
    public:
        //virtual void wait_for(CommandBuffer* dependency) = 0;
        //virtual void wait_for(std::span<CommandBuffer*> dependencies) = 0;
        
        //virtual Renderpass* begin_renderpass() = 0;
        //virtual void end_renderpass(Renderpass* renderpass) = 0;
    };
    
    class Device
    {
    protected:
        Device() = default;

    public:
        virtual ~Device() = default;
        
        virtual bool supports_advanced_sync() = 0;
        
        virtual bool create_swapchain(SDL_Window* window)  = 0;
        virtual void destroy_swapchain(SDL_Window* window) = 0;

        virtual CommandBuffer* begin_commandbuffer(CommandQueue queue) = 0;
        virtual void end_commandbuffer(CommandBuffer* cmd) = 0;
        
        virtual bool submit_commandbuffers(std::span<CommandBuffer*> cmds) = 0;
        bool submit_commandbuffer(CommandBuffer* cmd) { return submit_commandbuffers({&cmd, 1}); }
        
        virtual std::string get_name() const = 0;
    };

    [[nodiscard]] Device* create_metal_device(bool debug = false);
    [[nodiscard]] Device* create_vulkan_device(DeviceLUID luid, bool debug = false);

    // Da implementare dopo perche' d3d12 mi sta altamente sul cazzo
    // [[nodiscard]] Device* create_d3d12_device(DeviceLUID luid, bool debug = false, bool gpu_va = false);

} // namespace core::rhi
