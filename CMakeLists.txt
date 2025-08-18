cmake_minimum_required(VERSION 3.16)
project(OpenGLProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# GLAD
add_library(glad src/glad.c)
target_include_directories(glad PUBLIC include)

# GLFW (manually specify paths)
include_directories(/opt/homebrew/include)
link_directories(/opt/homebrew/lib)

# Main executable
add_executable(opengl_raycast src/main.cpp)
target_link_libraries(opengl_raycast PRIVATE glad glfw)

# Apple frameworks
if(APPLE)
    target_link_libraries(opengl_raycast PRIVATE "-framework Cocoa" "-framework OpenGL" "-framework IOKit")
endif()