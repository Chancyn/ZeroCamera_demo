#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "nng::nng" for configuration ""
set_property(TARGET nng::nng APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(nng::nng PROPERTIES
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libnng.so.1.7.3"
  IMPORTED_SONAME_NOCONFIG "libnng.so.1"
  )

list(APPEND _IMPORT_CHECK_TARGETS nng::nng )
list(APPEND _IMPORT_CHECK_FILES_FOR_nng::nng "${_IMPORT_PREFIX}/lib/libnng.so.1.7.3" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
