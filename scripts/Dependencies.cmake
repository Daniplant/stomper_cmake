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
        NAME Vulkan-Utility-Libraries
        GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Utility-Libraries
        GIT_TAG v1.4.327)

CPMAddPackage(
        NAME Vulkan-Headers
        VERSION 1.4.327
        URL https://github.com/KhronosGroup/Vulkan-Headers/archive/refs/tags/v1.4.327.zip
        URL_HASH SHA256=908a4fd186f00fffa513c0235df9c1c7c8a7283521d04fdb002bcde0b344d77d)

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
            NAME DirectX-Headers
            VERSION 1.616.0
            URL https://github.com/microsoft/DirectX-Headers/archive/refs/tags/v1.616.0.zip
            URL_HASH SHA256=09e9c218d04fe34e1f12c21ec8188983a034e223b8fbfb3ec2ab1573dd03c39e)

    CPMAddPackage(
            NAME DirectX-AgilitySDK
            VERSION 1.616.1
            URL https://www.nuget.org/api/v2/package/Microsoft.Direct3D.D3D12/1.616.0
            URL_HASH SHA256=2c7b31f0e41192673ace986506c65b7cc7520610cfe5f30216152905e67ff5ba
            DOWNLOAD_ONLY TRUE)
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
        add_library(Microsoft::DirectX12-Agility INTERFACE IMPORTED)
        set_target_properties(Microsoft::DirectX12-Agility PROPERTIES INTERFACE_LINK_LIBRARIES "Microsoft::DirectX12-Core;Microsoft::DirectX12-Layers")
    endif()
endif()