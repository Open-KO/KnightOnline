add_library(JpegFile STATIC
  JpegFile.cpp
  JpegFile.h
)

set_target_properties(JpegFile PROPERTIES
  CXX_SCAN_FOR_MODULES OFF
)

# Enable warnings as errors
if(OPENKO_COMPILE_WARNINGS_AS_ERROR)
  set_property(TARGET JpegFile PROPERTY COMPILE_WARNING_AS_ERROR ON)
endif()

# Setup dependencies
target_link_libraries(JpegFile PUBLIC libjpeg-turbo)

# Expose include path
target_include_directories(JpegFile PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}/../" # so we can access <JpegFile/...>
)

# Setup warning levels
target_compile_options(JpegFile PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W3>
  $<$<CXX_COMPILER_ID:GNU>:-Wall>
  $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra>
)
