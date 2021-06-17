project(
  atug2gtest
  VERSION 1.0
  DESCRIPTION "gtest for atug2"
  LANGUAGES CXX
)

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY "https://github.com/google/googletest.git"
  GIT_TAG        release-1.10.0
  SOURCE_DIR ${CMAKE_SOURCE_DIR}/_deps/googletest
)
FetchContent_MakeAvailable(googletest)
# set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# source to binary
set(SOURCES 
  testmain.cc
  # test_data.cc
  # test_logfile.cc
  test_sharedptr.cc
  test_comm.cc
  test_threadroutine.cc
)
add_executable(${PROJECT_NAME} ${SOURCES})
target_compile_options(${PROJECT_NAME} PUBLIC /Wall)

# target - this project
# includes, definitions, options
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_BINARY_DIR}/_deps/googletest-src/googletest/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_BINARY_DIR}/_deps/googletest-src/googletest/include/gtest)
# to link
set(LIBS
  atug2
)
# link targets, gtest to project
target_link_libraries(${PROJECT_NAME} 
  ${LIBS} gtest
)
