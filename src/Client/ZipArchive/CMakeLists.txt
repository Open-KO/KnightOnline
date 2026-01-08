find_package(MFC REQUIRED)

# MFC (static)
set(CMAKE_MFC_FLAG 1)

add_library(ZipArchive STATIC
  StdAfx.h
  ZipArchive.cpp
  ZipArchive.h
  ZipAutoBuffer.cpp
  ZipAutoBuffer.h
  ZipCentralDir.cpp
  ZipCentralDir.h
  ZipException.cpp
  ZipException.h
  ZipFileHeader.cpp
  ZipFileHeader.h
  ZipInternalInfo.cpp
  ZipInternalInfo.h
  ZipStorage.cpp
  ZipStorage.h)

# Enable warnings as errors
if(OPENKO_COMPILE_WARNINGS_AS_ERROR)
  set_property(TARGET ZipArchive PROPERTY COMPILE_WARNING_AS_ERROR ON)
endif()

# Setup PCH
target_precompile_headers(ZipArchive PRIVATE StdAfx.h)

# Setup dependencies
target_link_libraries(ZipArchive PUBLIC
  libzlib
)

# Expose include path
target_include_directories(ZipArchive INTERFACE
  "${CMAKE_CURRENT_SOURCE_DIR}/../" # so we can access <ZipArchive/...>
)

# Setup warning levels
# target_compile_options(ZipArchive PRIVATE
#  $<$<CXX_COMPILER_ID:MSVC>:/W3>
#  $<$<CXX_COMPILER_ID:GNU>:-Wall>
#  $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra>
#)
