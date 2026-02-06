# We use a user-modifiable config file for the server files named server_config.h
# This is copied from the default if it doesn't already exist.
set(DEFAULT_SERVER_CONFIG_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/server_config.h.default")
set(TARGET_SERVER_CONFIG_DIR "${CMAKE_CURRENT_BINARY_DIR}/shared-server")
set(TARGET_SERVER_CONFIG_HEADER "${TARGET_SERVER_CONFIG_DIR}/server_config.h")

# Make sure the target directory exists
file(MAKE_DIRECTORY "${TARGET_SERVER_CONFIG_DIR}")

if(NOT EXISTS "${TARGET_SERVER_CONFIG_HEADER}")
  configure_file("${DEFAULT_SERVER_CONFIG_HEADER}" "${TARGET_SERVER_CONFIG_HEADER}" COPYONLY)
endif()

add_library(shared-server STATIC
  _USER_DATA.h
  AppThread.cpp
  AppThread.h
  logger.cpp
  logger.h
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
  STLMap.h
  TcpClientSocket.cpp
  TcpClientSocket.h
  TcpClientSocketManager.cpp
  TcpClientSocketManager.h
  TcpServerSocket.cpp
  TcpServerSocket.h
  TcpServerSocketManager.cpp
  TcpServerSocketManager.h
  TcpSocket.cpp
  TcpSocket.h
  TcpSocketManager.cpp
  TcpSocketManager.h
  TelnetThread.h
  TelnetThread.cpp
  TelnetClientThread.h
  TelnetClientThread.cpp
  utilities.cpp
  utilities.h
  "${TARGET_SERVER_CONFIG_HEADER}"
)

set_target_properties(shared-server PROPERTIES
  CXX_SCAN_FOR_MODULES OFF
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
    db-library
    djb2
	MathUtils
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
