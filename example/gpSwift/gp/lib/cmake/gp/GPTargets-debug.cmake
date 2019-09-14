#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "gpproto::gpnet" for configuration "Debug"
set_property(TARGET gpproto::gpnet APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(gpproto::gpnet PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libgpnet.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS gpproto::gpnet )
list(APPEND _IMPORT_CHECK_FILES_FOR_gpproto::gpnet "${_IMPORT_PREFIX}/lib/libgpnet.a" )

# Import target "gpproto::gputils" for configuration "Debug"
set_property(TARGET gpproto::gputils APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(gpproto::gputils PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libgputils.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS gpproto::gputils )
list(APPEND _IMPORT_CHECK_FILES_FOR_gpproto::gputils "${_IMPORT_PREFIX}/lib/libgputils.a" )

# Import target "gpproto::gpclient" for configuration "Debug"
set_property(TARGET gpproto::gpclient APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(gpproto::gpclient PROPERTIES
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libgpclient.dylib"
  IMPORTED_SONAME_DEBUG "@rpath/libgpclient.dylib"
  )

list(APPEND _IMPORT_CHECK_TARGETS gpproto::gpclient )
list(APPEND _IMPORT_CHECK_FILES_FOR_gpproto::gpclient "${_IMPORT_PREFIX}/lib/libgpclient.dylib" )

# Import target "gpproto::gpclientstatic" for configuration "Debug"
set_property(TARGET gpproto::gpclientstatic APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(gpproto::gpclientstatic PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libgpclientstatic.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS gpproto::gpclientstatic )
list(APPEND _IMPORT_CHECK_FILES_FOR_gpproto::gpclientstatic "${_IMPORT_PREFIX}/lib/libgpclientstatic.a" )

# Import target "gpproto::gpcore" for configuration "Debug"
set_property(TARGET gpproto::gpcore APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(gpproto::gpcore PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libgpcore.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS gpproto::gpcore )
list(APPEND _IMPORT_CHECK_FILES_FOR_gpproto::gpcore "${_IMPORT_PREFIX}/lib/libgpcore.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
