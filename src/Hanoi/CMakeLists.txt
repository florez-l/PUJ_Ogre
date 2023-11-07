## =========================================================================
## @author Leonardo Florez-Valencia (florez-l@javeriana.edu.co)
## =========================================================================

subdirs(resources)

include_directories(
  ${PROJECT_BINARY_DIR}/lib
  ${PROJECT_SOURCE_DIR}/lib
  )

set(_t)
if(WIN32)
  set(_t WIN32)
elseif(APPLE)
  set(_t MACOSX_BUNDLE)
endif()

add_executable(Hanoi ${_t} Hanoi.cxx)
target_link_libraries(Hanoi PUJ_Ogre)

## eof - CMakeLists.txt
