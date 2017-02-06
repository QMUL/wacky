cmake_minimum_required(VERSION 3.0.0)

PROJECT(wacky)

# Basic includes
include_directories( "include" )

# Boost for Memory Mapping the big files
FIND_PACKAGE( Boost 1.58 COMPONENTS program_options system filesystem REQUIRED )
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )


# Options (gcc mostly) 
SET(CMAKE_CXX_FLAGS "-std=c++11 -static-libstdc++")
SET(CMAKE_CXX_FLAGS_DEBUG "-g -std=c++11 -static-libstdc++")

SET(CMAKE_CXX_FLAGS_PROFILE "-pg")
SET(CMAKE_C_FLAGS_PROFILE "-pg")

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")

ADD_EXECUTABLE(wacky src/wacky.cc src/wacky_create.cc src/wacky_math.cc src/wacky_read.cc src/wacky_sbj_obj.cc src/wacky_verb.cc src/wacky_breakup.cc)

target_link_libraries(wacky ${Boost_LIBRARIES}) 

