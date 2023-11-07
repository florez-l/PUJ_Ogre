## =========================================================================
## @author Leonardo Florez-Valencia (florez-l@javeriana.edu.co)
## =========================================================================

include_directories(
  ${BULLET_INCLUDE_DIRS}
  ${PROJECT_BINARY_DIR}/lib
  ${PROJECT_SOURCE_DIR}/lib
  )
add_library(
  PUJ_Ogre SHARED
  BaseApplication.cxx BaseApplication_VTK.cxx BaseApplication.h
  CameraMan.cxx CameraMan.h
  )
generate_export_header(
  PUJ_Ogre
  EXPORT_MACRO_NAME PUJ_Ogre_EXPORT
  EXPORT_FILE_NAME Export.h
  )
target_link_libraries(
  PUJ_Ogre
  stdc++fs
  ${VTK_LIBRARIES}
  ${BULLET_LIBRARIES}
  OgreMain OgreBites
  )

## eof - CMakeLists.txt
