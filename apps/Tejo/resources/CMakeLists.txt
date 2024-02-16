## =========================================================================
## @author Leonardo Florez-Valencia (florez-l@javeriana.edu.co)
## =========================================================================

file(
  ARCHIVE_CREATE
  OUTPUT PUJ_Ogre_Tejo_resources.zip
  PATHS "cancha.obj" "material.material" "sphere.obj"
  FORMAT zip
  VERBOSE
  )

## eof - CMakeLists.txt
