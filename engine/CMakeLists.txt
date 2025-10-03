project(Engine LANGUAGES C CXX VERSION 1.0.0)

file(GLOB ENGINE_SOURCES
	${PROJECT_SOURCE_DIR}/src/rhi/*.h
	${PROJECT_SOURCE_DIR}/src/rhi/*.c
	${PROJECT_SOURCE_DIR}/src/rhi/*.hpp 
	${PROJECT_SOURCE_DIR}/src/rhi/*.cpp
	${PROJECT_SOURCE_DIR}/src/rhi/vulkan/*.h
	${PROJECT_SOURCE_DIR}/src/rhi/vulkan/*.c
	${PROJECT_SOURCE_DIR}/src/rhi/vulkan/*.hpp
	${PROJECT_SOURCE_DIR}/src/rhi/vulkan/*.cpp)

file(GLOB_RECURSE ENGINE_HEADERS
    ${PROJECT_SOURCE_DIR}/include/*.h
    ${PROJECT_SOURCE_DIR}/include/*.hpp)

if(APPLE)
	file(GLOB APPLE_ENGINE_SOURCES ${PROJECT_SOURCE_DIR}/src/rhi/metal/*.hpp ${PROJECT_SOURCE_DIR}/src/rhi/metal/*.mm)
	list(APPEND ENGINE_SOURCES ${APPLE_ENGINE_SOURCES})
endif()

if(WIN32)
	file(GLOB WIN32_ENGINE_SOURCES ${PROJECT_SOURCE_DIR}/src/rhi/d3d12/*.hpp ${PROJECT_SOURCE_DIR}/src/rhi/d3d12/*.cpp)
	list(APPEND ENGINE_SOURCES ${WIN32_ENGINE_SOURCES})
endif()

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${ENGINE_HEADERS} ${ENGINE_SOURCES})

add_library(Engine STATIC ${ENGINE_SOURCES} ${ENGINE_HEADERS})
target_include_directories(Engine PRIVATE ${PROJECT_SOURCE_DIR}/src PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(Engine PUBLIC
		volk::volk
		cereal::cereal
		spdlog::spdlog
		SDL3::SDL3-static
		Vulkan::Headers
		Vulkan::UtilityHeaders
		$<$<PLATFORM_ID:Windows>:Microsoft::DirectX12-Agility>)

if(APPLE)
    target_link_libraries(Engine PUBLIC "-framework Foundation" "-framework Metal" "-framework QuartzCore")
    target_compile_options(Engine PUBLIC -fobjc-arc)
endif()

target_compile_definitions(Engine PUBLIC VK_NO_PROTOTYPES)

if(SDL_CPU_X64)
    target_compile_definitions(Engine PUBLIC CEREAL_XML_STRING_VALUE="heap" "$<$<CXX_COMPILER_ID:Clang,AppleClang>:-mavx2;-mfma>")
endif()
