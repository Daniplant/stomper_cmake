include(${CMAKE_CURRENT_LIST_DIR}/CPM.cmake)

CPMAddPackage(
        NAME SDL3
        VERSION 3.2.22
        URL https://github.com/libsdl-org/SDL/releases/download/release-3.2.22/SDL3-3.2.22.zip
        URL_HASH SHA256=3d60068b1e5c83c66bb14c325dfef46f8fcc380735b4591de6f5e7b9738929d1
        OPTIONS "SDL_AUDIO OFF" "SDL_GPU OFF" "SDL_RENDERER OFF" "SDL_SHARED OFF" "SDL_STATIC ON")

CPMAddPackage(
        NAME spdlog
        VERSION 1.15.3
        URL https://github.com/gabime/spdlog/archive/refs/tags/v1.15.3.zip
        URL_HASH SHA256=b74274c32c8be5dba70b7006c1d41b7d3e5ff0dff8390c8b6390c1189424e094)

CPMAddPackage(
        NAME volk
        GIT_REPOSITORY https://github.com/zeux/volk
        GIT_TAG vulkan-sdk-1.4.328)

if(WIN32)
    set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WIN32_KHR)
elseif(LINUX)
    set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WAYLAND_KHR)
elseif(APPLE)
    set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_METAL_EXT)
endif()

CPMAddPackage(
        NAME Vulkan-Utility-Libraries
        VERSION 1.4.328
        URL https://github.com/KhronosGroup/Vulkan-Utility-Libraries/archive/refs/tags/v1.4.328.zip
        URL_HASH SHA256=139f42eb06dc98e1853de37f01035142ea8221247a13bae2d197275bf21156d1)

CPMAddPackage(
        NAME Vulkan-Headers
        VERSION 1.4.328
        URL https://github.com/KhronosGroup/Vulkan-Headers/archive/refs/tags/v1.4.328.zip
        URL_HASH SHA256=75fe4d82d41381229ea7dea2892e3344f500fd5de62e8994f79f4979f432a261)

CPMAddPackage(
        NAME cereal
        VERSION 1.3.2
        URL https://github.com/USCiLab/cereal/archive/refs/tags/v1.3.2.zip
        URL_HASH SHA256=e72c3fa8fe3d531247773e346e6824a4744cc6472a25cf9b30599cd52146e2ae
        OPTIONS "BUILD_DOC OFF" "BUILD_SANDBOX OFF" "SKIP_PERFORMANCE_COMPARISON ON")

CPMAddPackage(
        NAME catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2
        GIT_TAG v3.10.0
        GIT_SHALLOW TRUE)
        
if(WIN32)
    CPMAddPackage(
            NAME DirectX-AgilitySDK
            VERSION 1.715.1-preview
            URL https://www.nuget.org/api/v2/package/Microsoft.Direct3D.D3D12/1.715.1-preview
            URL_HASH SHA256=7c0fd2eba933a3f57426eb8510225ddcb2f41ef929eaaf1fec6e2894c6c22bde
            DOWNLOAD_ONLY TRUE)

     CPMAddPackage(
            NAME DirectX-Headers
            VERSION 1.715.0-preview
            URL https://github.com/microsoft/DirectX-Headers/archive/refs/tags/v1.715.0-preview.zip
            URL_HASH SHA256=0186cc094daf6763b2535261dc2d13671d9efebe90e27c6cdbead7c214197f7f)

    if(DirectX-AgilitySDK_ADDED)
        find_library(D3D12_LIB NAMES d3d12 REQUIRED)

        add_library(Microsoft::DirectX12-Core SHARED IMPORTED)
        set_target_properties(Microsoft::DirectX12-Core PROPERTIES
                IMPORTED_LOCATION_RELEASE            "${DirectX-AgilitySDK_SOURCE_DIR}/build/native/bin/x64/D3D12Core.dll"
                IMPORTED_LOCATION_DEBUG              "${DirectX-AgilitySDK_SOURCE_DIR}/build/native/bin/x64/D3D12Core.dll"
                IMPORTED_IMPLIB                      "${D3D12_LIB}"
                IMPORTED_CONFIGURATIONS              "Debug;Release"
                IMPORTED_LINK_INTERFACE_LANGUAGES    "C")

        add_library(Microsoft::DirectX12-Layers SHARED IMPORTED)
        set_target_properties(Microsoft::DirectX12-Layers PROPERTIES
                IMPORTED_LOCATION_RELEASE            "${DirectX-AgilitySDK_SOURCE_DIR}/build/native/bin/x64/d3d12SDKLayers.dll"
                IMPORTED_LOCATION_DEBUG              "${DirectX-AgilitySDK_SOURCE_DIR}/build/native/bin/x64/d3d12SDKLayers.dll"
                IMPORTED_IMPLIB                      "${D3D12_LIB}"
                IMPORTED_CONFIGURATIONS              "Debug;Release"
                IMPORTED_LINK_INTERFACE_LANGUAGES    "C")

        add_library(Microsoft::DirectSR SHARED IMPORTED)
        set_target_properties(Microsoft::DirectSR PROPERTIES
                IMPORTED_LOCATION_RELEASE            "${DirectX-AgilitySDK_SOURCE_DIR}/build/native/bin/x64/DirectSR.dll"
                IMPORTED_LOCATION_DEBUG              "${DirectX-AgilitySDK_SOURCE_DIR}/build/native/bin/x64/DirectSR.dll"
                IMPORTED_IMPLIB                      "${D3D12_LIB}"
                IMPORTED_CONFIGURATIONS              "Debug;Release"
                IMPORTED_LINK_INTERFACE_LANGUAGES    "C")

        add_library(Microsoft::DirectX12-Agility INTERFACE IMPORTED)

        set_target_properties(Microsoft::DirectX12-Agility PROPERTIES INTERFACE_LINK_LIBRARIES "Microsoft::DirectX12-Core;Microsoft::DirectX12-Layers;Microsoft::DirectSR;Microsoft::DirectX-Guids;Microsoft::DirectX-Headers;dxgi;dxguid")
    endif()
endif()