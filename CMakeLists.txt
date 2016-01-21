cmake_minimum_required(VERSION 3.2.2)
project(batchtool)

# flags
set(CXX_FLAGS "-std=c++11")

# headers
include_directories(include)

# libraries
include_directories(lib)
set(LIBS lib/pugixml/src/pugixml.cpp lib/lodepng/lodepng.cpp)

file(GLOB SOURCES "src/*.cpp")

SET( CMAKE_CXX_FLAGS  "${CXX_FLAGS}" )
add_executable(batchtool ${LIBS} ${SOURCES})
