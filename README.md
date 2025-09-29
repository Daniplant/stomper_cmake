# Stomper 🐾
## A modern game engine

Mind you that this is an extremely early WIP, nothing works properly and nothing is written well

### Requirements
- CMake 3.30+
- Clang 16.0+ on Linux/MacOS
- Visual Studio 17+ 2022 on Windows
- VulkanSDK for Validation Layers
- A Vulkan 1.2-capable GPU for all platforms, or Metal3 on MacOS

## How to build
`cmake -S root/of/repo -B root/of/repo/build && cmake --build root/of/repo/build`

## I don't see any validation layer messages
Keep the Vulkan configurator open and they should work
