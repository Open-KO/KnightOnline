# We use a user-modifiable config file for the server files named server_config.h
# This is copied from the default if it doesn't already exist.
set(DEFAULT_CONFIG_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/server_config.h.default")
set(TARGET_CONFIG_DIR "${CMAKE_CURRENT_BINARY_DIR}/shared-server")
set(TARGET_CONFIG_HEADER "${TARGET_CONFIG_DIR}/server_config.h")

# Make sure the target directory exists
file(MAKE_DIRECTORY "${TARGET_CONFIG_DIR}")

if(NOT EXISTS "${TARGET_CONFIG_HEADER}")
    configure_file("${DEFAULT_CONFIG_HEADER}" "${TARGET_CONFIG_HEADER}" COPYONLY)
endif()

add_library(shared-server STATIC
  _USER_DATA.h
  AppThread.cpp
  AppThread.h
  ftxui_sink_mt.cpp
  ftxui_sink_mt.h
  GeometricStructs.h
  logger.cpp
  logger.h
  Matrix44.inl
  My_3DStruct.cpp
  My_3DStruct.h
  N3ShapeMgr.cpp
  N3ShapeMgr.h
  pch.h
  ReadQueueThread.cpp
  ReadQueueThread.h
  SharedMemoryBlock.cpp
  SharedMemoryBlock.h
  SharedMemoryQueue.cpp
  SharedMemoryQueue.h
  SocketManager.cpp
  SocketManager.h
  STLMap.h
  TcpClientSocket.cpp
  TcpClientSocket.h
  TcpServerSocket.cpp
  TcpServerSocket.h
  TcpSocket.cpp
  TcpSocket.h
  Vector3.inl
  utilities.cpp
  utilities.h
  "${TARGET_CONFIG_HEADER}"
)

# Enable warnings as errors
if(OPENKO_COMPILE_WARNINGS_AS_ERROR)
  set_property(TARGET shared-server PROPERTY COMPILE_WARNING_AS_ERROR ON)
endif()

# Setup PCH
target_precompile_headers(shared-server PRIVATE pch.h)

# Setup dependencies
target_link_libraries(shared-server
  PUBLIC
    argparse
    asio
    djb2
    ftxui::component
    ftxui::dom
    ftxui::screen
    nanodbc
    shared
  PRIVATE
    Boost::interprocess
)

# Expose include path
target_include_directories(shared-server PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}/../" # so we can access <shared-server/...>
  "${CMAKE_CURRENT_BINARY_DIR}/"    # we copied server_config.h here, under "shared-server", so we should be able to access it too
)

# Setup warning levels
target_compile_options(shared-server PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W3>
  $<$<CXX_COMPILER_ID:GNU>:-Wall>
  $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra>
)
