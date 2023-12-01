cmake_minimum_required(VERSION 3.8)

# Project name
project(fog) 

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 11)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

include_directories(extern/imgui-docking)
file(GLOB imgui_src "extern/imgui-docking/*.cpp")
file(GLOB imgui_backends
        "extern/imgui-docking/backends/imgui_impl_glfw.cpp"
        "extern/imgui-docking/backends/imgui_impl_glfw.h"
        "extern/imgui-docking/backends/imgui_impl_opengl3.cpp"
        "extern/imgui-docking/backends/imgui_impl_opengl3.h"
        "extern/imgui-docking/backends/imgui_impl_opengl3_loader.h"
        "extern/imgui-docking/misc/cpp/imgui_stdlib.cpp"
        )

find_package(OpenGL REQUIRED COMPONENTS OpenGL)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)

add_executable(${PROJECT_NAME} main.cpp Perlin.cpp TextureGenerator.cpp ${imgui_src} ${imgui_backends})
# add_executable(${PROJECT_NAME} main.cpp Perlin.cpp stb_image.cpp)

target_link_libraries(${PROJECT_NAME} OpenGL::GL GLEW::glew glfw)