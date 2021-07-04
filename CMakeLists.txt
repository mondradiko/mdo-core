cmake_minimum_required(VERSION 3.13)
include(cmake/prelude.cmake)
project(mdo-core VERSION 0.0.0)

# options
option(COMPILE_SHADERS "Compile shaders in bulid tree.")
option(TRACY_ENABLE "Enable Tracy profiling.")

if(MSVC)
  set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
  set(BUILD_SHARED_LIBS TRUE)
 endif()

include_directories(include)

# dependencies
#include(FindVulkan)
#if(NOT Vulkan_FOUND)
# message(FATAL_ERROR "Vulkan not found.")
#endif()
#if(NOT Vulkan_GLSLC_EXECUTABLE)
#  message(FATAL_ERROR "glslc not found.")
#endif()

find_mondradiko_dependency(
  mondradiko::vulkan "Vulkan" NOCONFIG
  INSTALL "vulkan-sdk"
  Vulkan::Vulkan
)

find_mondradiko_dependency(
  mondradiko::sdl2 "SDL2" SDL2::SDL2main
  INSTALL "sdl2[vulkan]"
  UNIX SDL2::SDL2-static
  # UNIX SDL2::SDL2
  WIN32 SDL2::SDL2
)

# shaders
function(spirv_shader ret GLSL VAR)
  set(SPIRV "${CMAKE_CURRENT_BINARY_DIR}/${GLSL}.spv")
  set(GLSL "${CMAKE_CURRENT_SOURCE_DIR}/${GLSL}")

  add_custom_command(
    OUTPUT ${SPIRV}
    COMMAND Vulkan::glslc -O -o ${SPIRV} ${GLSL}
    DEPENDS ${GLSL}
  )

  set(${ret} "${SPIRV}" PARENT_SCOPE)
endfunction(spirv_shader)

function(spirv_shaders ret)
  foreach(GLSL ${ARGN})
    string(MAKE_C_IDENTIFIER ${GLSL} IDENTIFIER)
    spirv_shader(HEADER ${GLSL} ${IDENTIFIER})
    list(APPEND HEADERS ${HEADER})
  endforeach()

  set(${ret} "${HEADERS}" PARENT_SCOPE)
endfunction(spirv_shaders)

set(SHADERS_SRC
  shaders/debug.frag
  shaders/debug.vert
)

if(COMPILE_SHADERS)
  spirv_shaders(SHADERS_BIN ${SHADERS_SRC})
  add_library(mdo-shaders INTERFACE ${SHADERS_BIN})
else()
  message(WARNING "Shader compilation is disabled.")
  message(WARNING "Ask Marceline on Discord for a .zip of the current shaders.")
endif()

# main library
set(MDO_CORE_SRC
  src/displays/sdl/sdl_display.c
  src/gpu/gpu_device.c
  src/gpu/gpu_shader.c
  src/gpu/gpu_vector.c
  src/renderer/debug/debug_draw.c
  src/renderer/debug/debug_pass.c
  src/renderer/camera.c
  src/renderer/renderer.c
  src/renderer/viewport.c
)

if(TRACY_ENABLE)
  list(APPEND MDO_CORE_SRC lib/Tracy/TracyClient.cpp)
  include_directories(lib/Tracy/)
  add_compile_definitions(TRACY_ENABLE)
endif()

add_library(mdo-core SHARED ${MDO_CORE_SRC})

target_link_libraries(mdo-core
  mondradiko::sdl2
  mondradiko::vulkan
)

add_executable(mdo-cli cli/main.c)
target_link_libraries(mdo-cli mdo-core)

