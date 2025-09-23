project(Tests LANGUAGES C CXX VERSION 1.0.0)

file(GLOB_RECURSE TESTS_SOURCES
		${PROJECT_SOURCE_DIR}/src/*.cpp)

add_executable(Tests ${TESTS_SOURCES})
target_link_libraries(Tests PRIVATE Engine Catch2::Catch2WithMain)