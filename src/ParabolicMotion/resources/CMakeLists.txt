## =========================================================================
## @author Leonardo Florez-Valencia (florez-l@javeriana.edu.co)
## =========================================================================

get_filename_component(_b "${CMAKE_CURRENT_SOURCE_DIR}" NAME)
get_filename_component(_p "${CMAKE_CURRENT_SOURCE_DIR}" PATH)
get_filename_component(_a "${_p}" NAME)

file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/${_a}_${_b}")

file(GLOB _r "*")
foreach(_f ${_r})
  get_filename_component(_n "${_f}" NAME)
  if(NOT ${_n} STREQUAL "CMakeLists.txt")
    configure_file("${_n}" "${PROJECT_BINARY_DIR}/${_a}_${_b}" COPYONLY)
  endif()
endforeach()

file(ARCHIVE_CREATE
  OUTPUT "${_a}_${_b}.zip"
  PATHS  "${PROJECT_BINARY_DIR}/${_a}_${_b}"
  FORMAT "zip"
  )

## eof - CMakeLists.txt
