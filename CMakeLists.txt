cmake_minimum_required(VERSION 3.19)

# solution ver set
set(atug2_VERSION_MAJOR 1)
set(atug2_VERSION_MINOR 0)
set(atug2_VERSION_PATCH 0)
set(atug2_VERSION_TWEAK 0)
project(
  atug2
  VERSION ${atug2_VERSION_MAJOR}.${atug2_VERSION_MINOR}.${atug2_VERSION_PATCH}.${atug2_VERSION_TWEAK}
  DESCRIPTION "atug2; a totally understandable good 'second work' for c++ development"
  LANGUAGES CXX
)
configure_file(${CMAKE_SOURCE_DIR}/lib_info.h.in ${CMAKE_SOURCE_DIR}/lib_info.h @ONLY)


# common compiler settings
set(CMAKE_CXX_STANDARD 17)


# compile options
set(COMPILE_DEFINITIONS )
# platform specific
if (MSVC)
  message("HOST_ARCHITECTURE: ${CMAKE_VS_PLATFORM_TOOLSET_HOST_ARCHITECTURE}")
  set(TARGET_ARCH ${CMAKE_VS_PLATFORM_TOOLSET_HOST_ARCHITECTURE}) # x86, x64, ...
  message("##### ${PROJECT_NAME}'s TARGET_ARCH + BUILD_TYPE : ${TARGET_ARCH} + ${CMAKE_CONFIGURATION_TYPES}")
  add_compile_options(/Wall /W4) # /WX -> all warings are errors
  set(CMAKE_CXX_FLAGS_DEBUG "/Zi /Ob0 /Od /RTC1") # /ZI	Includes debug information in a program database compatible with Edit and Continue. (x86 only)
  set(CMAKE_CXX_FLAGS_RELEASE "/O2 /Ob2 /DNDEBUG") # /O2	Creates fast code.
  set(CMAKE_CXX_FLAGS_MINSIZEREL "/O1 /Ob1 /DNDEBUG") # /Os	Favors small code.
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/Zi /O2 /Ob1 /DNDEBUG") # /Zi	Generates complete debugging information.

  # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4819 /wd4820")
  # add_compile_options(/volatile ...)
  # target_compile_definitions(${PROJECT_NAME} PUBLIC _WINDOWS ATUG2_EXPORTS)
  set(COMPILE_DEFINITIONS _WINDOWS _MSVC UNICODE ATUG2_EXPORTS)
elseif (UNIX)
  set(CMAKE_CXX_FLAGS "-O1 -Wall")
  set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
  set(CMAKE_CXX_FLAGS_RELEASE "-O2")
  set(CMAKE_CXX_FLAGS_MINSIZEREL "-Os")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-Og -g")
endif ()

# output dir for library, binary - synced with Visual Studio
if (MSVC)
  set(OUTPUT_DIR ${CMAKE_SOURCE_DIR}/out/msvc)
elseif (UNIX)
  set(OUTPUT_DIR ${CMAKE_SOURCE_DIR}/out/unix)
endif ()
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${TARGET_ARCH}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${TARGET_ARCH}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${TARGET_ARCH}/bin)

if (${CMAKE_CONFIGURATION_TYPES} STREQUAL "Debug")
  set(COMPILE_DEFINITIONS ${COMPILE_DEFINITIONS} _DEBUG)
  
endif ()
add_compile_definitions(${COMPILE_DEFINITIONS})



# library src
set(SRC
  _src/adata.cc
  _src/afile_attr.cc
  _src/afile_win_impl.cc
  _src/afile.cc
  _src/ainstance_limiter.cc
  _src/asafelist.cc
  _src/alog.cc
  _src/amemory.cc
  _src/aroutine.cc
  _src/ashared_ptr.cc
  _src/asocket.cc
  _src/asocket_comm_routine.cc
  _src/athread.cc
  _src/atime.cc
  _src/acrc.cc
)
add_library(${PROJECT_NAME} SHARED ${SRC})
target_include_directories(${PROJECT_NAME} PUBLIC 
  include
)


# dependencies
# add_subdirectory()

# external libraries
# include(FetchContent)

# tests
add_subdirectory(_tests)




