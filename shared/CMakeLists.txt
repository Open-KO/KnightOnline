add_library(shared STATIC
  ByteBuffer.h
  CircularBuffer.h
  crc32.cpp
  crc32.h
  DateTime.cpp
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

# Use C++20
target_compile_features(shared PUBLIC cxx_std_20)

# Setup PCH
target_precompile_headers(shared PRIVATE pch.h)

# These files shouldn't use PCH
set_source_files_properties(
  crc32.cpp
  lzf.cpp
  PROPERTIES SKIP_PRECOMPILE_HEADERS ON
)

# Setup dependencies
target_link_libraries(shared
  PUBLIC spdlog::spdlog # expose spdlog to consumers
)
