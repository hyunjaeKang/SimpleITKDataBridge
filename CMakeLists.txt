cmake_minimum_required(VERSION 3.2.2)
project(SimpleITKDataBridge)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/CMake)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpic")

include_directories ( ${PYTHON_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} )

set(CMAKE_SWIG_FLAGS ${CMAKE_SWIG_GLOBAL_FLAGS} -features autodoc=1 -keyword )
set(CMAKE_SWIG_OUTDIR ${CMAKE_CURRENT_BINARY_DIR})

# SimpleITK
find_package(SimpleITK REQUIRED)
include(${SimpleITK_USE_FILE})
include_directories(${SimpleITK_INCLUDE_DIRS})

# SWIG
find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})
set(CMAKE_SWIG_FLAGS "")

# Python
find_package(PythonLibs)
include_directories(${PYTHON_INCLUDE_PATH})

#NumPy
find_package(NumPy REQUIRED)
include_directories(${NUMPY_INCLUDE_DIR})


# local files
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

#find_package ( PythonLibs REQUIRED )
#find_package ( PythonInterp REQUIRED )
#include_directories ( ${PYTHON_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} )

# Run swig
set(CMAKE_SWIG_FLAGS ${CMAKE_SWIG_GLOBAL_FLAGS} -features autodoc=1 -keyword )

set_source_files_properties ( SimpleITK.i PROPERTIES CPLUSPLUS ON )

SWIG_add_module( SimpleITK python SimpleITK.i sitkPyCommand.cxx )
SWIG_LINK_LIBRARIES(SimpleITK ${PYTHON_LIBRARIES} ${SimpleITK_LIBRARIES})

#ADD_LIBRARY(SimpleITK sitkPyCommand.cxx)
#LINK_LIBRARIES(SimpleITK ${PYTHON_LIBRARIES} ${PYADD_LIBRARY} ${ITK_LIBRARIES} ${SimpleITK_LIBRARIES})
