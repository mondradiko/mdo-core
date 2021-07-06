Outlined below is a detailed exploration of building the Mondradiko toolchain on different platforms:

# x86_x64 (w/ vcpkg)

You need to run the "x64 Native Tools Command Prompt," remove your build directory, and configure cmake with that specific 
command prompt and a clean build directory, that way the proper windows compiler ecosystem gets used by vcpkg.

What's happening, in instances where vcpkg bootstraps but fails to run, is that vcpkg is trying to run, but then silently 
failing to install the proper dependencies because of compiler issues; if the cmake commandline tool isn't available with 
that prompt, you can install it with Chocolatey.

# x86_x64 (w/ POSIX Layer)

Generally, install vcpkg, and cmake to your favorite application for this purpose(cygwin, mingw64, WSL-Ubuntu). Clone the mdo-core
directory to your home folder, cd into the folder and use cmake -s home/(src) -b home/(src)/build. Be sure to set USE_LAZY_VCPKG to
TRUE before building otherwise you will need each library installed locally (though this boils down to trivial apt-get and install 
calls if you neglect the use of vcpkg in such an environment)

# x86_64 (w/o vcpkg, POSIX Layer AKA Manually w/ CMake)

**Before you start: Install the VulkanSDK.**

You will need to download the source of the following Libraries/sdks (in addition to the VulkanSDK):

mdo-core(mdo-cli, mdo-utils):

1. SDL2
2. libuv
3. openxr(additonally needs jsoncpp)
4. flecs
5. cmocka (.exe Installation is fine)

We then approach the process sequentially by each program listed with an mdo suffix:

## mdo-core
Download the source, open it with CMake, and configure it with COMPILE_SHADERS, CMAKE_USE_WIN32_THREADS_INIT, VCPKG_PREFER_SYSTEM_LIBS,
and X_VCPKG_APP_LOCAL_DEPS_INSTALL set to true and USE_LAZY_VCPKG set to false; Configure.

Create directories in root for each library to put their source in:

- SDL2
- libuv
- openxr (additonally create a jsoncpp dir in the openxr dir)
- flecs 
- cmocka 
- tracy

set each (Library_name)__DIR__ variable to each library directory in root, Configure; confirm your Shaders folder contains debug.frag, debug.vert, 
be sure to ask Marceline for the latest .SPV shaders (join our discord!) Configure, Generate.

### Potential Issues/Side-Steps:
Generate may fail with some known errors, here's how to handle them in order:

*cmocka* - 
Even when downloading cmocka from the appropriate website it's release is apparently packaged as a 32-bit library which, as of 2019, will throw
an error in cmake that is really warning you of an error VS/XCode will throw when trying to build a 64-bit application against non-x64 source.

There is no workaround to this on Windows or Mac short of simply building the library locally; move all built source to the mdo-core/lib folder afterward.  

*SDL2* - 
When not using vcpkg, you will have to build SDL2 using cmake. Configure the in folder sdl2 directory with cmake and be sure to move it's 
SDL2Targets.cmake (it will be under Build/CMake) to the same folder as SDL2-Config.cmake. mdo-core's SDL2_DIR_ must contain both.

*libuv* - 
Normally, vcpkg will download unofficial-libuv::libuv and build it locally. As a workaround you can use [dacap/libuv-cmake](https://github.com/dacap/libuv-cmake)
follow his readme and put the built uv.lib in the mdo-core/libs folder 

*openxr* - 
OpenXR needs JsonCpp to fully function, download it's source and use cmake to configure and generate OpenXR's VS Solution. Take JsonCpp source's Include folder and copy it
to the mdo-core/include folder, name it json.

*tracy* - 
Create a "Tracy" folder in mdo-core/include rather than root, set TRACY_ENABLE to true in CMake and be sure to clone the TracyC git directory into the mdo-core/include/tracy folder.

## mdo-cli
Requires all the libraries in question and a mdo-core.lib reference. When mdo-core finishes building take the .lib file and place it in mdo-core/lib.

## mdo-utils
Requires a reference to mdo-config.cmake.in, and mdo-utils-config.cmake use mdo-core/build as your MDO_CORE_DIR. mdo-config.cmake.in will appear in the mdo-utils/cmake folder.

### Visual Studio Steps
After having generated the necessary files with CMake you will need to change some Project Properties.

Linking: Change the Additional Libraries directory to {LOCAL_SYSTEM}/mdo-core/lib, and to the location of the VulkanSDK (alternatively, copy Vulkan's .h headers 
into the following path: mdo-core/include/vulkan)

Input: Change Each Library entry to one that points to mdo-core/lib (ex: mdo-core/lib/uv.lib)

WARNING: On Win32, OpenXR will only build OpenXR_Loader.dll and it's demos (hello-xr, conformance, etc).
For this reason you can remove the Input reference to openxr-all-supported.lib. This will be addressed in the future.

Build the Solution, be sure to create a final folder in the location of mdo-cli.exe called "shaders" and place the .SPV shaders inside; if all done correctly congratulations!
you have manually built the Mondradiko Toolchain on Win32!

# i386 (ARMv7/v8)
TBA (cmake)
