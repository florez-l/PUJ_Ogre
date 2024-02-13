## =========================================================================
## @author Leonardo Florez-Valencia (florez-l@javeriana.edu.co)
## =========================================================================

add_executable(PUJ_Ogre_OBJViewer main.cxx)
target_link_libraries(
  PUJ_Ogre_OBJViewer
  stdc++fs gmp
  OgreMain OgreBites
  )

## eof - CMakeLists.txt
