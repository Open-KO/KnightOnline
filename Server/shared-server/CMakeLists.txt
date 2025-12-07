# We use a user-modifiable config file for the server files named server_config.h
# This is copied from the default if it doesn't already exist.
set(DEFAULT_CONFIG_HEADER server_config.h.default)
set(TARGET_CONFIG_HEADER server_config.h)

if(NOT EXISTS ${TARGET_CONFIG_HEADER})
    configure_file(${DEFAULT_CONFIG_HEADER} ${TARGET_CONFIG_HEADER} COPYONLY)
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
  My_3DStruct.h
  N3ShapeMgr.cpp
  N3ShapeMgr.h
  pch.h
  ReadQueueThread.cpp
  ReadQueueThread.h
  server_config.h
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
  utilities.cpp
  utilities.h
)

# Use C++20
target_compile_features(shared-server PUBLIC cxx_std_20)

# Setup PCH
target_precompile_headers(shared-server PRIVATE pch.h)

# Setup dependencies
target_link_libraries(shared-server
  PUBLIC
    asio
    shared
    nanodbc
    ftxui::component
  PRIVATE
    Boost::interprocess
)

# Expose include path
target_include_directories(
  shared-server PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../
)
