cmake_minimum_required(VERSION 3.10)
project(web-browser)

file(GLOB SOURCES "src/*.cpp")

find_package(SDL3 REQUIRED CONFIG)
find_package(SDL3_ttf REQUIRED CONFIG)

add_subdirectory(vendor/gumbo)

add_executable(web-browser ${SOURCES})

target_link_libraries(web-browser PRIVATE 
    SDL3::SDL3
    SDL3_ttf::SDL3_ttf
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/liblitehtml.a
    gumbo
)

if(WIN32)
    target_link_libraries(web-browser PRIVATE ws2_32)
endif()

target_include_directories(web-browser PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    vendor/gumbo-parser/include
)

target_compile_definitions(web-browser PRIVATE SDL_MAIN_HANDLED)

if(WIN32)
    set(SDL3_DLL_DIR "${CMAKE_SOURCE_DIR}/vendor")
    add_custom_command(TARGET web-browser POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${SDL3_DLL_DIR}/SDL3.dll"
            "$<TARGET_FILE_DIR:web-browser>"
    )
endif()
