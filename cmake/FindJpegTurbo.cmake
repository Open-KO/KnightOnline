# Get JpegTurbo package
#
# Makes the libjpeg-turbo target available.

include(ExternalProject)

if(MSVC)
  set(LIBJPEG_TURBO_STATIC_SUFFIX "-static")
  set(LIBJPEG_TURBO_ENABLE_SHARED OFF)
  set(LIBJPEG_TURBO_ENABLE_STATIC ON)
else()
  set(LIBJPEG_TURBO_STATIC_SUFFIX "")
  set(LIBJPEG_TURBO_ENABLE_SHARED ON)
  set(LIBJPEG_TURBO_ENABLE_STATIC OFF)
endif()

ExternalProject_Add(
  libjpeg-turbo-project
  GIT_REPOSITORY   "https://github.com/libjpeg-turbo/libjpeg-turbo.git"
  GIT_TAG          "3.1.3"
  GIT_PROGRESS     ON
  GIT_SHALLOW      ON
  PREFIX           "${CMAKE_CURRENT_BINARY_DIR}/libjpeg-turbo"
  INSTALL_DIR      "${CMAKE_CURRENT_BINARY_DIR}/libjpeg-turbo"
  BUILD_BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/libjpeg-turbo/lib/${CMAKE_STATIC_LIBRARY_PREFIX}jpeg${LIBJPEG_TURBO_STATIC_SUFFIX}${CMAKE_STATIC_LIBRARY_SUFFIX}"
  CMAKE_CACHE_ARGS
    -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
    -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
    -DCMAKE_INSTALL_LIBDIR:PATH=<INSTALL_DIR>/lib
    -DWITH_TOOLS:BOOL=OFF
    -DENABLE_SHARED:BOOL=${LIBJPEG_TURBO_ENABLE_SHARED}
    -DENABLE_STATIC:BOOL=${LIBJPEG_TURBO_ENABLE_STATIC}
)

ExternalProject_Get_Property(libjpeg-turbo-project install_dir)

set(LIBJPEG_TURBO_INCLUDE_DIR "${install_dir}/include")
set(LIBJPEG_TURBO_LIBRARY "${install_dir}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}jpeg${LIBJPEG_TURBO_STATIC_SUFFIX}${CMAKE_STATIC_LIBRARY_SUFFIX}")

# Create the directory so setting it doesn't fail
file(MAKE_DIRECTORY ${LIBJPEG_TURBO_INCLUDE_DIR})

add_library(libjpeg-turbo STATIC IMPORTED)
set_target_properties(libjpeg-turbo PROPERTIES
  IMPORTED_LOCATION "${LIBJPEG_TURBO_LIBRARY}"
)
add_dependencies(libjpeg-turbo libjpeg-turbo-project)

target_include_directories(libjpeg-turbo INTERFACE
  "${LIBJPEG_TURBO_INCLUDE_DIR}"
)

set(jpegturbo_FOUND TRUE)
