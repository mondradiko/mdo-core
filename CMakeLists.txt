cmake_minimum_required(VERSION 3.13)
project(mdo-core VERSION 0.0.0)

include_directories(include)

# dependencies
find_package(SDL2 REQUIRED)

include(FindVulkan)
if(NOT Vulkan_FOUND)
  message(FATAL_ERROR "Vulkan not found.")
endif()

# main library
set(MDO_CORE_SRC
  src/displays/sdl/sdl_display.c
  src/gpu/gpu_device.c
  src/renderer/camera.c
  src/renderer/viewport.c
)

add_library(mdo-core SHARED ${MDO_CORE_SRC})

target_link_libraries(mdo-core
  SDL2::SDL2
  Vulkan::Vulkan
)

add_executable(mdo-cli cli/main.c)
target_link_libraries(mdo-cli mdo-core)

