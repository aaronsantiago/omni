SET(Pistache_INSTALL_DIR "/usr") 

find_path(Pistache_INCLUDE_DIR router.h PATHS
  ${Pistache_INSTALL_DIR}/include/pistache
  ${Pistache_INSTALL_DIR}/include
)

find_library(Pistache_LIBRARY pistache PATH ${Pistache_INSTALL_DIR}/lib/x86_64-linux-gnu)

if (Pistache_INCLUDE_DIR AND Pistache_LIBRARY)
  SET(Pistache_INCLUDE_DIRS ${Pistache_INCLUDE_DIR})
  SET(Pistache_LIBRARIES ${Pistache_LIBRARY})
  SET(Pistache_VERSION "0.5")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Pistache
                                  REQUIRED_VARS Pistache_LIBRARY Pistache_INCLUDE_DIR
                                  VERSION_VAR Pistache_VERSION)

mark_as_advanced(Pistache_INCLUDE_DIRS Pistache_LIBRARIES)
