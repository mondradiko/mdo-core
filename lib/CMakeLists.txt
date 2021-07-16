# options
option(FLECS_PRECOMPILE_HEADER "Precompile flecs.h. Improves build times. Only works in CMake >=3.16" ON)

# Fix Linux relocatable linking
# Only applies to targets in this scope
if(UNIX)
  add_compile_options(-fPIC)
endif()

# CGLM
set(CGLM_SHARED OFF)
set(CGLM_STATIC ON)
add_subdirectory(cglm)

# Tracy
if(TRACY_ENABLE)
  add_library(tracy STATIC Tracy/TracyClient.cpp)
  target_compile_definitions(tracy PUBLIC TRACY_ENABLE)
else()
  add_library(tracy INTERFACE)
endif()

target_include_directories(tracy INTERFACE Tracy)

# FLECS
add_library(flecs STATIC flecs/flecs.c)
target_include_directories(flecs PUBLIC flecs)

# libuv
set(LIBUV_BUILD_TESTS OFF)
add_subdirectory(libuv)

# volk
add_library(volk STATIC volk/volk.c)
target_include_directories(volk PUBLIC volk)
target_compile_definitions(volk PUBLIC VK_NO_PROTOTYPES)

if(FLECS_PRECOMPILE_HEADER AND ${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.16")
  target_precompile_headers(flecs PUBLIC flecs/flecs.h)
endif()
