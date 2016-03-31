cmake_minimum_required(VERSION 3.2.2)
project(SimpleITKDataBridge)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpic")

include_directories ( ${PYTHON_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} )

set(CMAKE_SWIG_FLAGS ${CMAKE_SWIG_GLOBAL_FLAGS} -features autodoc=1 -keyword )
set(CMAKE_SWIG_OUTDIR ${CMAKE_CURRENT_BINARY_DIR})

# SimpleITK
find_package(SimpleITK REQUIRED)
include(${SimpleITK_USE_FILE})
INCLUDE_DIRECTORIES(${SimpleITK_INCLUDE_DIRS})

# SWIG
find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})
set(CMAKE_SWIG_FLAGS "")

# Python
find_package(PythonLibs)
INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_PATH})

# local files
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

#find_package ( PythonLibs REQUIRED )
#find_package ( PythonInterp REQUIRED )
#include_directories ( ${PYTHON_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} )

# Run swig
set(CMAKE_SWIG_FLAGS ${CMAKE_SWIG_GLOBAL_FLAGS} -features autodoc=1 -keyword )

set_source_files_properties ( SimpleITKDataBridge.i PROPERTIES CPLUSPLUS ON )

SWIG_add_module( SimpleITKDataBridge python SimpleITKDataBridge.i sitkPyCommand.cxx )
SWIG_LINK_LIBRARIES(SimpleITKDataBridge ${PYTHON_LIBRARIES} ${SimpleITK_LIBRARIES})

#ADD_LIBRARY(SimpleITKDataBridge sitkPyCommand.cxx)
#LINK_LIBRARIES(SimpleITK ${PYTHON_LIBRARIES} ${PYADD_LIBRARY} ${ITK_LIBRARIES} ${SimpleITK_LIBRARIES})
