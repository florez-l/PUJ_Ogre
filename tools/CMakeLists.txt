## =========================================================================
## @author Leonardo Florez-Valencia (florez-l@javeriana.edu.co)
## =========================================================================

add_executable(PUJ_Ogre_ConvertVTKToMesh ConvertVTKToMesh.cxx)
target_link_libraries(PUJ_Ogre_ConvertVTKToMesh ${VTK_LIBRARIES} OgreMain)

## eof - CMakeLists.txt
