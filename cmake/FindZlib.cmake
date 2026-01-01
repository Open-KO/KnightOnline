# Get Zlib package
#
# Makes the libzlib target available.

fetchcontent_declare(
  zlib
  GIT_REPOSITORY        "https://github.com/madler/zlib.git"
  GIT_TAG               "v1.3.1.2"
  GIT_PROGRESS          ON
  GIT_SHALLOW           ON
  EXCLUDE_FROM_ALL
)

set(ZLIB_BUILD_TESTING OFF CACHE BOOL "zlib: Enable Zlib Examples as tests")
set(ZLIB_INSTALL ON CACHE BOOL "zlib: Enable installation of zlib")

if(MSVC)
  set(ZLIB_BUILD_SHARED OFF CACHE BOOL "zlib: Enable building zlib shared library")
endif()

fetchcontent_makeavailable(zlib)

# Setup a wrapper project to reference the preferred zlib target for this build
add_library(libzlib INTERFACE)
if(MSVC)
  target_link_libraries(libzlib INTERFACE zlibstatic)
else()
  target_link_libraries(libzlib INTERFACE zlib)
endif()

set(zlib_FOUND TRUE)
