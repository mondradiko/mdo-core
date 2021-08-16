cmake_minimum_required(VERSION 3.13)
project(mdo-core VERSION 0.0.0)

# options
option(COMPILE_SHADERS "Enable shader compilation." ON)
option(TRACY_ENABLE "Enable Tracy profiling.")

if(MSVC)
  set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
  set(BUILD_SHARED_LIBS TRUE)
endif()

include_directories(include)

# dependencies
include(FindVulkan)
#if(NOT Vulkan_FOUND)
#  message(FATAL_ERROR "Vulkan not found.")
#endif()

find_program(GLSL_COMPILER glslc NO_CACHE)
if(GLSL_COMPILER) # glslc args
  set(GLSL_ARGS "-O")
else() # glslangValidator args
  message(WARNING "glslc not found, using slower glslangValidator instead")
  find_program(GLSL_COMPILER glslangValidator NO_CACHE)
  set(GLSL_ARGS "-V")
endif()

if(COMPILE_SHADERS AND NOT GLSL_COMPILER)
  message(FATAL_ERROR "Suitable shader compiler not found.")
endif()

add_subdirectory(lib)

find_package(SDL2 REQUIRED)
if(TARGET SDL2::SDL2)  # support targeted SDL2 distributions
  set(SDL2_LIBRARIES SDL2::SDL2 SDL2::SDL2main)
endif()

find_package(OpenXR REQUIRED)

# shaders
function(spirv_shader ret GLSL VAR)
  set(SPIRV "${CMAKE_CURRENT_BINARY_DIR}/${GLSL}.spv")
  set(GLSL "${CMAKE_CURRENT_SOURCE_DIR}/${GLSL}")

  add_custom_command(
    OUTPUT ${SPIRV}
    COMMAND ${GLSL_COMPILER} ${GLSL_ARGS} ${GLSL} -o ${SPIRV}
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
  add_subdirectory(shaders)
  add_library(mdo-shaders INTERFACE ${SHADERS_BIN})
endif()

# main library
set(MDO_CORE_SRC
  src/displays/sdl/sdl_display.c
  src/displays/openxr/openxr_display.c
  src/gpu/gpu_device.c
  src/gpu/gpu_shader.c
  src/gpu/gpu_vector.c
  src/renderer/debug/debug_draw.c
  src/renderer/debug/debug_pass.c
  src/renderer/camera.c
  src/renderer/renderer.c
  src/renderer/viewport.c
  src/network/network_client.c
  src/network/network_server.c
  src/world/world.c
  src/log.c
)

add_library(mdo-core SHARED ${MDO_CORE_SRC})

target_link_libraries(mdo-core
  ${SDL2_LIBRARIES}
  OpenXR::openxr_loader
  flecs
  cglm
  tracy
  uv_a
  volk
)

add_executable(mdo-cli cli/main.c)
target_link_libraries(mdo-cli mdo-core)
