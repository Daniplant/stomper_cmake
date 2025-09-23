cmake_minimum_required(VERSION 3.30)
project(Heap)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(APPLE)
    enable_language(OBJCXX)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_OBJCXX_STANDARD 20)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CPM_SOURCE_CACHE ${CMAKE_SOURCE_DIR}/.cpm_cache)

include(${CMAKE_SOURCE_DIR}/scripts/Functions.cmake)
include(${CMAKE_SOURCE_DIR}/scripts/Dependencies.cmake)

add_subdirectory(engine)
add_subdirectory(editor)
add_subdirectory(tests)

get_all_targets(DEPS_TARGETS)
set_target_properties(
        ${DEPS_TARGETS}
        PROPERTIES
        FOLDER "Dependencies")

set_target_properties(
        Tests
        Engine
        Editor
        PROPERTIES
        FOLDER "Runtime")
