cmake_minimum_required(VERSION 3.0.2)
message("--------------------------------------------------------------------------------")

project(ReflaxMan)

set(CMAKE_CONFIGURATION_TYPES "Debug" "Release")

if (WIN32)
  file(GLOB SRC src/windows/*.* src/common/*.*)
endif (WIN32)

if (UNIX)
  file(glob SRC src/linux/*.* src/common/*.*)
endif (UNIX)

add_executable(ReflaxMan ${SRC})

set_target_properties(ReflaxMan PROPERTIES RUNTIME_OUTPUT_DIRECTORY "../build")
set_target_properties(ReflaxMan PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "../build")
set_target_properties(ReflaxMan PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "../build")

if (WIN32)
  if (MSVC)
    set_target_properties(ReflaxMan PROPERTIES DEBUG_POSTFIX "_debug")
    set(CMAKE_CXX_FLAGS_DEBUG "/W3 /Gm /MT /Zi /Od")
    set(CMAKE_CXX_FLAGS_RELEASE "/Gm /MT /Zi /O2 /Oi /Ot /GL /fp:fast /DNDEBUG")
    set(CMAKE_EXE_LINKER_FLAGS "/MACHINE:X86 /DYNAMICBASE:NO /MANIFEST:NO /SUBSYSTEM:WINDOWS")
  else(MSVC)
    set(CMAKE_CXX_COMPILER g++)
    set_target_properties(ReflaxMan PROPERTIES DEBUG_POSTFIX "_debug")
    set(CMAKE_CXX_FLAGS_DEBUG "-Wall -g -Wno-multichar")
    set(CMAKE_CXX_FLAGS_RELEASE "-Wno-multichar -fexpensive-optimizations -Ofast")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "-mwindows -static -lgdi32")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-mwindows -static -lgdi32 -s")
  endif(MSVC)
endif (WIN32)

if (UNIX)
    set(CMAKE_CXX_COMPILER g++)
    set_target_properties(ReflaxMan PROPERTIES DEBUG_POSTFIX "_debug")
    set(CMAKE_CXX_FLAGS "-Wno-multichar -fexpensive-optimizations -Ofast")
    set(CMAKE_CXX_FLAGS_DEBUG "-Wall -g -Wno-multichar")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "-lX11")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-lX11 -s")
endif (UNIX)



