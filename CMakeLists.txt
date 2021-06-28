cmake_minimum_required(VERSION 3.13)
project(mdo-core VERSION 0.0.0)

include_directories(include)

# dependencies
find_package(SDL2 REQUIRED)

# main library
set(MDO_CORE_SRC
  src/displays/sdl/sdl_display.c
  src/gpu/gpu_device.c
)

add_library(mdo-core SHARED ${MDO_CORE_SRC})

target_link_libraries(mdo-core
  SDL2::SDL2
)

add_executable(mdo-cli cli/main.c)
target_link_libraries(mdo-cli mdo-core)

