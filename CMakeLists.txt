cmake_minimum_required(VERSION 3.0.0)

PROJECT(wacky)

# Basic includes
include_directories( "include" )

# Boost for Memory Mapping the big files
FIND_PACKAGE( Boost 1.58 COMPONENTS program_options system filesystem REQUIRED )
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )

# CUDA details
find_package(CUDA QUIET REQUIRED)
option(USE_CUDA "Use CUDA for doing the math" NO)

# MKL
option(USE_MKL "Use the MKL Intel Library for the th math" NO)

# Options (gcc mostly) 
SET(CMAKE_CXX_FLAGS "-std=c++11 -static-libstdc++")
SET(CMAKE_CXX_FLAGS_DEBUG "-g -std=c++11 -static-libstdc++")

SET(CMAKE_CXX_FLAGS_PROFILE "-pg")
SET(CMAKE_C_FLAGS_PROFILE "-pg")

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")

if (USE_CUDA)
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_USE_CUDA")
  set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS} -D_FORCE_INLINES -O3 -gencode arch=compute_52,code=sm_52)
  CUDA_ADD_EXECUTABLE(wacky src/wacky.cc src/wacky_create.cc src/wacky_math.cc src/wacky_read.cc src/wacky_sbj_obj.cc src/wacky_verb.cc src/wacky_breakup.cc src/cuda_verb.cu src/cuda_math.cu)
  target_link_libraries(wacky ${Boost_LIBRARIES}) 
else()

  if (USE_MKL)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_USE_MKL")
    INCLUDE_DIRECTORIES( ${MKL_INCLUDE_DIR} )
    ADD_EXECUTABLE(wacky src/wacky.cc src/wacky_create.cc src/wacky_math.cc src/wacky_read.cc src/wacky_sbj_obj_mkl.cc src/wacky_verb.cc src/wacky_breakup.cc)
    target_link_libraries(wacky ${Boost_LIBRARIES} ${MKL_LIBRARY_PATH}/libmkl_core.a)

  else()
    ADD_EXECUTABLE(wacky src/wacky.cc src/wacky_create.cc src/wacky_math.cc src/wacky_read.cc src/wacky_sbj_obj.cc src/wacky_verb.cc src/wacky_breakup.cc)
    target_link_libraries(wacky ${Boost_LIBRARIES}) 
    ADD_EXECUTABLE(wacky_bench src/wacky_bench.cc src/wacky_math.cc src/wacky_sbj_obj.cc src/wacky_breakup.cc)
    target_link_libraries(wacky_bench ${Boost_LIBRARIES}) 
endif()



