cmake_minimum_required(VERSION 3.16)
project(learnvulkan)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# App
add_executable(learnvulkan src/main.cpp)

include(FetchContent)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.3
    GIT_SHALLOW TRUE
)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
    GIT_SHALLOW TRUE
)
# fetch optimizations
set(FETCHCONTENT_QUIET OFF)
set(BUILD_EXAMPLES OFF CACHE INTERNAL "")
FetchContent_MakeAvailable(glfw glm)

# Vulkan (system SDK)
find_package(Vulkan REQUIRED)

target_link_libraries(learnvulkan
    PRIVATE
    Vulkan::Vulkan
    glm::glm-header-only
    glfw
)