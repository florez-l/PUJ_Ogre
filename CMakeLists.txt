## =========================================================================
## @author Leonardo Florez-Valencia (florez-l@javeriana.edu.co)
## =========================================================================

cmake_minimum_required(VERSION 3.20)
project(PUJ_Tejo)

## -- Some configurations
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

## -- Packages
find_package(VTK REQUIRED)
find_package(OGRE REQUIRED)

## -- Targets
subdirs(tools)

## eof - CMakeLists.txt
