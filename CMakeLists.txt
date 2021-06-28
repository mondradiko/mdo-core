cmake_minimum_required(VERSION 3.13)
project(mdo-core VERSION 0.0.0)

include_directories(include)

set(MDO_CORE_SRC
	src/displays/sdl/sdl_display.c
)

add_library(mdo-core SHARED ${MDO_CORE_SRC})

add_executable(hello-world src/main.c)

add_executable(mdo-cli cli/main.c)

