cmake_minimum_required(VERSION 3.13)
project(mdo-core VERSION 0.0.0)

include_directories(include)

# dependencies
find_package(SDL2 REQUIRED)

include(FindVulkan)
if(NOT Vulkan_FOUND)
  message(FATAL_ERROR "Vulkan not found.")
endif()
if(NOT Vulkan_GLSLC_EXECUTABLE)
  message(FATAL_ERROR "glslc not found.")
endif()

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

spirv_shaders(SHADERS_BIN ${SHADERS_SRC})
add_library(mdo-shaders INTERFACE ${SHADERS_BIN})

# main library
set(MDO_CORE_SRC
  src/displays/sdl/sdl_display.c
  src/gpu/gpu_device.c
  src/gpu/gpu_shader.c
  src/gpu/gpu_vector.c
  src/renderer/debug/debug_pass.c
  src/renderer/camera.c
  src/renderer/renderer.c
  src/renderer/viewport.c
)

add_library(mdo-core SHARED ${MDO_CORE_SRC})

target_link_libraries(mdo-core
  SDL2::SDL2
  Vulkan::Vulkan
)

add_executable(mdo-cli cli/main.c)
target_link_libraries(mdo-cli mdo-core)

