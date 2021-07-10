# CGLM
#set(CGLM_SHARED OFF)
#set(CGLM_STATIC ON)
#add_subdirectory(cglm)

# Tracy
if(TRACY_ENABLE)
  add_library(tracy STATIC Tracy/TracyClient.cpp)
  target_compile_definitions(tracy PUBLIC TRACY_ENABLE)

  if(UNIX)
    target_compile_options(tracy PRIVATE -fPIC)
  endif()
else()
  add_library(tracy INTERFACE)
endif()

target_include_directories(tracy INTERFACE Tracy)
