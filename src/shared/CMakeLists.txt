add_library(shared STATIC
  ByteBuffer.cpp
  ByteBuffer.h
  CircularBuffer.cpp
  CircularBuffer.h
  crc32.cpp
  crc32.h
  DateTime.cpp
  ExpandableCircularBuffer.cpp
  ExpandableCircularBuffer.h
  globals.cpp
  globals.h
  Ini.cpp
  Ini.h
  JvCryption.cpp
  JvCryption.h
  lzf.cpp
  lzf.h
  Packet.h
  packets.h
  pch.h
  StringUtils.cpp
  StringUtils.h
  Thread.cpp
  Thread.h
  TimerThread.cpp
  TimerThread.h
  version.h
)

set_target_properties(shared PROPERTIES
  CXX_SCAN_FOR_MODULES OFF
)

# Enable warnings as errors
if(OPENKO_COMPILE_WARNINGS_AS_ERROR)
  set_property(TARGET shared PROPERTY COMPILE_WARNING_AS_ERROR ON)
endif()

# Setup PCH
target_precompile_headers(shared PRIVATE pch.h)

# These files shouldn't use PCH
set_source_files_properties(
  crc32.cpp
  lzf.cpp
  PROPERTIES SKIP_PRECOMPILE_HEADERS ON
)

# Setup dependencies
target_link_libraries(shared PUBLIC
  spdlog::spdlog
)

# Setup warning levels
target_compile_options(shared PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W3>
  $<$<CXX_COMPILER_ID:GNU>:-Wall>
  $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra>
)
