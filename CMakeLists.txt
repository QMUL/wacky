cmake_minimum_required(VERSION 3.0.0)

PROJECT(wacky)

# Basic includes
include_directories( "include" )

# Boost for Memory Mapping the big files and testing
# TODO - Tests dont work with boost 1.58 sadly :(
FIND_PACKAGE( Boost 1.58 COMPONENTS program_options system filesystem unit_test_framework REQUIRED )
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )

# Custom Options
option(USE_CUDA "Use CUDA for doing the math" NO)
option(USE_MKL "Use the MKL Intel Library for the math" NO)

# Options (gcc mostly) 
SET(CMAKE_CXX_FLAGS "-std=c++11 -static-libstdc++")
SET(CMAKE_CXX_FLAGS_DEBUG "-g -std=c++11 -static-libstdc++ -D_GLIBCXX_USE_CXX11_ABI=0")

SET(CMAKE_CXX_FLAGS_PROFILE "-pg -std=c++11 -static-libstdc++")
SET(CMAKE_C_FLAGS_PROFILE "-pg -std=c++11 static-libstdc++")

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")

# CUDA Version
if (USE_CUDA)

  find_package(CUDA QUIET REQUIRED)
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_USE_CUDA")
  set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS} -D_FORCE_INLINES -O3 -gencode arch=compute_52,code=sm_52)
  CUDA_ADD_EXECUTABLE(wacky src/wacky.cc src/wacky_create.cc src/wacky_math.cc src/wacky_read.cc src/wacky_sbj_obj.cc src/wacky_verb.cc src/wacky_breakup.cc src/cuda_verb.cu src/cuda_math.cu)
  target_link_libraries(wacky ${Boost_LIBRARIES}) 

else()

  # MKL Version
  if (USE_MKL)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_USE_MKL")
    find_path(MKL_INCLUDE_PATH mkl.h PATHS /opt/intel/mkl/include/)
    
    if (MKL_INCLUDE_PATH)
      INCLUDE_DIRECTORIES( ${MKL_INCLUDE_PATH} )
    else()
      message(FATAL_ERROR "Failed to find MKL Include Path")
    endif()

    ADD_EXECUTABLE(wacky src/wacky.cc src/wacky_create.cc src/wacky_math.cc src/wacky_read.cc src/wacky_sbj_obj_mkl.cc src/wacky_verb.cc src/wacky_breakup.cc)
    ADD_EXECUTABLE(wacky_bench src/wacky_bench.cc src/wacky_math.cc src/wacky_sbj_obj_mkl.cc src/wacky_breakup.cc)

    find_path(MKL_LIBRARY_PATH libmkl_core.a PATHS /opt/intel/mkl/lib/intel64_lin/)

    if (MKL_LIBRARY_PATH)
      target_link_libraries(wacky ${Boost_LIBRARIES} ${MKL_LIBRARY_PATH}/libmkl_rt.so)
      target_link_libraries(wacky_bench ${Boost_LIBRARIES} ${MKL_LIBRARY_PATH}/libmkl_rt.so) 
    else()
      message(FATAL_ERROR "Failed to find MKL Library Path")
    endif()
  # Basic version
  else()
    ADD_EXECUTABLE(wacky src/wacky.cc src/wacky_create.cc src/wacky_math.cc src/wacky_read.cc src/wacky_sbj_obj.cc src/wacky_verb.cc src/wacky_breakup.cc)
    target_link_libraries(wacky ${Boost_LIBRARIES}) 
    ADD_EXECUTABLE(wacky_bench src/wacky_bench.cc src/wacky_math.cc src/wacky_sbj_obj.cc src/wacky_breakup.cc)
    target_link_libraries(wacky_bench ${Boost_LIBRARIES}) 
  
  endif()

endif()

# Test bits
enable_testing()
if (USE_MKL)
	ADD_EXECUTABLE(wacky_test_basic test/basic.cc src/wacky_create.cc src/wacky_math.cc src/wacky_read.cc src/wacky_breakup.cc)
	add_test( basic wacky_test_basic)

	ADD_EXECUTABLE(wacky_test_verb test/verb.cc src/wacky_create.cc src/wacky_math.cc src/wacky_read.cc src/wacky_breakup.cc src/wacky_sbj_obj_mkl.cc src/wacky_verb.cc)
	add_test( verb wacky_test_basic)

	ADD_EXECUTABLE(wacky_test_math test/math.cc src/wacky_math.cc)
	add_test( wmath wacky_test_math)

	if (MKL_LIBRARY_PATH)
		target_link_libraries(wacky_test_basic ${Boost_LIBRARIES} ${MKL_LIBRARY_PATH}/libmkl_rt.so)
		target_link_libraries(wacky_test_verb ${Boost_LIBRARIES} ${MKL_LIBRARY_PATH}/libmkl_rt.so) 	
		target_link_libraries(wacky_test_math ${Boost_LIBRARIES} ${MKL_LIBRARY_PATH}/libmkl_rt.so) 
	else()
		message(FATAL_ERROR "Failed to find MKL Library Path")
	endif()


else()
	ADD_EXECUTABLE(wacky_test_basic test/basic.cc src/wacky_create.cc src/wacky_math.cc src/wacky_read.cc src/wacky_breakup.cc)
	target_link_libraries(wacky_test_basic ${Boost_LIBRARIES}) 
	add_test( basic wacky_test_basic)

	ADD_EXECUTABLE(wacky_test_verb test/verb.cc src/wacky_create.cc src/wacky_math.cc src/wacky_read.cc src/wacky_breakup.cc src/wacky_sbj_obj.cc src/wacky_verb.cc)
	target_link_libraries(wacky_test_verb ${Boost_LIBRARIES}) 
	add_test( verb wacky_test_basic)

	ADD_EXECUTABLE(wacky_test_math test/math.cc src/wacky_math.cc)
	target_link_libraries(wacky_test_math ${Boost_LIBRARIES}) 
	add_test( wmath wacky_test_math)

endif()

add_custom_command(TARGET wacky_test_basic PRE_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/test/output $<TARGET_FILE_DIR:wacky_test_basic>/output)

add_custom_command(TARGET wacky_test_basic PRE_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/test/ukwac $<TARGET_FILE_DIR:wacky_test_basic>/ukwac)

add_custom_command(TARGET wacky_test_verb PRE_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different
                       ${CMAKE_SOURCE_DIR}/test/simverb.txt $<TARGET_FILE_DIR:wacky_test_basic>)


