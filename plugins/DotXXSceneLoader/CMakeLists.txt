## =========================================================================
## @author Leonardo Florez-Valencia (florez-l@javeriana.edu.co)
## =========================================================================

#file(GLOB_RECURSE _common ${CMAKE_CURRENT_SOURCE_DIR}/Common/*)
#list(APPEND _common ${CMAKE_CURRENT_SOURCE_DIR}/Config.h)
#set(_common_libs Boost::boost)

#file(GLOB_RECURSE _eigen ${CMAKE_CURRENT_SOURCE_DIR}/eigen/*)
#set(_eigen_libs Eigen3::Eigen)

#file(GLOB_RECURSE _itk ${CMAKE_CURRENT_SOURCE_DIR}/ITK/*)
#set(_itk_libs ${ITK_LIBRARIES})

#if(ivq_USE_VTK)
#  file(GLOB_RECURSE _vtk ${CMAKE_CURRENT_SOURCE_DIR}/VTK/*)
#  set(_vtk_libs ${VTK_LIBRARIES})
#endif()

##file(GLOB_RECURSE _qt ${CMAKE_CURRENT_SOURCE_DIR}/QT/*)
##set(_qt_libs)

#set(_src "${_common};${_eigen};${_itk};${_vtk};${_qt}")
#set(
#  _libs
#  "${_common_libs};${_eigen_libs};${_itk_libs};${_vtk_libs};${_qt_libs}"
#  )

include_directories(
  ${PROJECT_BINARY_DIR}/plugins/DotXXSceneLoader
  ${PROJECT_SOURCE_DIR}/plugins/DotXXSceneLoader
  )
add_library(
  Plugin_PUJ_Ogre_DotXXSceneLoader
  SHARED
  PUJ_Ogre/DotXXSceneLoader.cxx PUJ_Ogre/DotXXSceneLoader.h
  PUJ_Ogre/DotXXSceneLoader_parametric2.cxx
  )
generate_export_header(
  Plugin_PUJ_Ogre_DotXXSceneLoader
  EXPORT_MACRO_NAME Plugin_PUJ_Ogre_DotXXSceneLoader_EXPORT
  EXPORT_FILE_NAME PUJ_Ogre/Plugin_PUJ_Ogre_DotXXSceneLoader_Export.h
  )
target_link_libraries(
  Plugin_PUJ_Ogre_DotXXSceneLoader
  PUBLIC OgreMain
  PRIVATE pugixml
  )
#ogre_config_framework(Plugin_PUJ_Ogre_DotXXSceneLoader)
#ogre_config_plugin(Plugin_PUJ_Ogre_DotXXSceneLoader)

## -- Installation rules
#install(
#  TARGETS ivq
#  EXPORT "${targets_export_name}"
#  LIBRARY DESTINATION "lib"
#  ARCHIVE DESTINATION "lib"
#  RUNTIME DESTINATION "bin"
#  INCLUDES DESTINATION "${include_install_dir}"
#  )

#set(_hdr ${_src})
#list(FILTER _hdr INCLUDE REGEX ".*\\.h.*$")
#foreach(_h ${_hdr})
#  get_filename_component(_d ${_h} DIRECTORY)
#  string(REGEX REPLACE "^${CMAKE_CURRENT_SOURCE_DIR}" "" _d ${_d})
#  ## string(REGEX REPLACE "^${CMAKE_CURRENT_BINARY_DIR}" "" _f ${_e})
#  install(FILES ${_h} DESTINATION include/ivq${_d})
#endforeach(_h)
#install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Export.h DESTINATION include/ivq)

## eof - CMakeLists.txt
