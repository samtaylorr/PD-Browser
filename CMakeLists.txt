cmake_minimum_required(VERSION 3.10)
project(web-browser)

# Find SDL3, SDL3_ttf and SDL3_image
find_package(SDL3 REQUIRED CONFIG)
find_package(SDL3_ttf REQUIRED CONFIG)
find_package(SDL3_image REQUIRED CONFIG)

# Add litehtml subdirectory (assumes you cloned it to lib/litehtml)
add_subdirectory(lib/litehtml)

# Collect all source files
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Create the executable
add_executable(web-browser ${SOURCES})

# Link dependencies
target_link_libraries(web-browser PRIVATE 
    SDL3::SDL3
    SDL3_ttf::SDL3_ttf
    SDL3_image::SDL3_image
    litehtml
)

# Include directories
target_include_directories(web-browser PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/litehtml/src
)

# Windows-specific settings
if(WIN32)
    target_link_libraries(web-browser PRIVATE ws2_32)
endif()

add_custom_command(TARGET web-browser POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/data/fonts"
        "${CMAKE_BINARY_DIR}/data/fonts"
)

# Prevent SDL from overriding main()
target_compile_definitions(web-browser PRIVATE SDL_MAIN_HANDLED)