# Get Jpeg package
#
# Makes the libjpeg target available.

fetchcontent_declare(
  jpeg
  GIT_REPOSITORY        "https://github.com/Open-KO/jpeg-cmake.git"
  GIT_TAG               "v1.3.0a-OpenKO"
  GIT_PROGRESS          ON
  GIT_SHALLOW           ON
  EXCLUDE_FROM_ALL
)

if(WIN32)
  set(LIBJPEG_BUILD_SHARED_LIBS OFF CACHE BOOL "libjpeg: Build shared libraries")
  set(LIBJPEG_BUILD_STATIC_LIBS ON CACHE BOOL "libjpeg: Build static libraries")
else()
  set(LIBJPEG_BUILD_SHARED_LIBS ON CACHE BOOL "libjpeg: Build shared libraries")
  set(LIBJPEG_BUILD_STATIC_LIBS OFF CACHE BOOL "libjpeg: Build static libraries")
endif()

set(LIBJPEG_INSTALL OFF CACHE BOOL "libjpeg: Install targets and headers")
set(LIBJPEG_BUILD_EXECUTABLES OFF CACHE BOOL "libjpeg: Build JPEG utilities")
set(LIBJPEG_BUILD_TESTS OFF CACHE BOOL "libjpeg: Build test executables")

fetchcontent_makeavailable(jpeg)

# Setup a wrapper project to reference the preferred jpeg target for this build
add_library(libjpeg INTERFACE)
if(WIN32)
  target_link_libraries(libjpeg INTERFACE jpeg_static)
else()
  target_link_libraries(libjpeg INTERFACE jpeg)
endif()

set(jpeg_FOUND TRUE)
