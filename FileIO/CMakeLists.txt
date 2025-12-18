add_library(FileIO STATIC
  File.h
  FileReader.cpp
  FileReader.h
  FileWriter.cpp
  FileWriter.h
)

# Enable warnings as errors
if(OPENKO_COMPILE_WARNINGS_AS_ERROR)
  set_property(TARGET FileIO PROPERTY COMPILE_WARNING_AS_ERROR ON)
endif()

# Setup dependencies
target_link_libraries(FileIO PUBLIC
  llfio_sl
)

# Setup warning levels
target_compile_options(FileIO PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W3>
  $<$<CXX_COMPILER_ID:GNU>:-Wall>
  $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra>
)
