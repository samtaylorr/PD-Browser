cmake_minimum_required(VERSION 3.10.0)
project(web-browser)

# Source files
file(GLOB SOURCES "src/*.cpp")

# Add executable
add_executable(web-browser ${SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32) 

# Include directories
target_include_directories(web-browser PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)