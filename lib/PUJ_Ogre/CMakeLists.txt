## =========================================================================
## @author Leonardo Florez-Valencia (florez-l@javeriana.edu.co)
## =========================================================================

include_directories(
  ${PROJECT_BINARY_DIR}/lib
  ${PROJECT_SOURCE_DIR}/lib
  )
set(
  _hdr
  BaseApplication.h
  Export.h
  OBJReader.h
  )
set(
  _src
  BaseApplication.cxx
  OBJReader.cxx
  )

add_library(PUJ_Ogre SHARED "${_hdr};${_src}")
generate_export_header(
  PUJ_Ogre
  EXPORT_MACRO_NAME PUJ_Ogre_EXPORT
  EXPORT_FILE_NAME Export.h
  )
target_link_libraries(
  PUJ_Ogre
  stdc++fs gmp
  OgreMain OgreBites
  )

## eof - CMakeLists.txt
