## =========================================================================
## @author Leonardo Florez-Valencia (florez-l@javeriana.edu.co)
## =========================================================================

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  )

set(_exec_type) #WIN32 MACOS
set(_libs stdc++fs) # TODO: if linux

add_executable(
  PUJ_Ogre_ConvertVTKToMesh ${_exec_type}
  Application.cxx Application.h main.cxx
  )
target_link_libraries(
  PUJ_Ogre_ConvertVTKToMesh
  ${_libs} OgreMain OgreBites ${VTK_LIBRARIES}
  )

#configure_file(
#  ${CMAKE_CURRENT_SOURCE_DIR}/resources.cfg
#  ${PROJECT_BINARY_DIR}/resources.cfg
#  @ONLY
#  )

#add_custom_target(PUJ_Tejo_models ALL
#  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/models ${PROJECT_BINARY_DIR}/models
#  DEPENDS PUJ_Tejo
#  )

## eof - CMakeLists.txt
