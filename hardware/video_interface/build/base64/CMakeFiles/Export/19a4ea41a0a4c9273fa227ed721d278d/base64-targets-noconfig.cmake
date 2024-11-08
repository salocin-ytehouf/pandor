#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "aklomp::base64" for configuration ""
set_property(TARGET aklomp::base64 APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(aklomp::base64 PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_NOCONFIG "C"
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libbase64.a"
  )

list(APPEND _cmake_import_check_targets aklomp::base64 )
list(APPEND _cmake_import_check_files_for_aklomp::base64 "${_IMPORT_PREFIX}/lib/libbase64.a" )

# Import target "aklomp::base64-bin" for configuration ""
set_property(TARGET aklomp::base64-bin APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(aklomp::base64-bin PROPERTIES
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/bin/base64"
  )

list(APPEND _cmake_import_check_targets aklomp::base64-bin )
list(APPEND _cmake_import_check_files_for_aklomp::base64-bin "${_IMPORT_PREFIX}/bin/base64" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
