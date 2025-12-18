add_executable(Ebenezer
  AISocket.cpp
  AISocket.h
  db_resources.h
  Define.h
  Ebenezer.cpp
  EbenezerApp.cpp
  EbenezerApp.h
  EbenezerLogger.cpp
  EbenezerLogger.h
  EbenezerReadQueueThread.cpp
  EbenezerReadQueueThread.h
  EbenezerResourceFormatter.cpp
  EbenezerResourceFormatter.h
  EbenezerSocketManager.cpp
  EbenezerSocketManager.h
  EVENT.cpp
  EVENT.h
  EVENT_DATA.cpp
  EVENT_DATA.h
  EXEC.cpp
  EXEC.h
  GameDefine.h
  GameEvent.cpp
  GameEvent.h
  Knights.cpp
  Knights.h
  KnightsManager.cpp
  KnightsManager.h
  KnightsSiegeWar.cpp
  KnightsSiegeWar.h
  LOGIC_ELSE.cpp
  LOGIC_ELSE.h
  MagicProcess.cpp
  MagicProcess.h
  Map.cpp
  Map.h
  Npc.cpp
  Npc.h
  OperationMessage.cpp
  OperationMessage.h
  pch.h
  RecvUDPThread.cpp
  RecvUDPThread.h
  Region.h
  resource.h
  SendWorkerThread.cpp
  SendWorkerThread.h
  UdpSocket.cpp
  UdpSocket.h
  User.cpp
  User.h
)

# Ensure we require the server assets to be copied
add_dependencies(AIServer copy-server-assets)

if(WIN32)
  target_sources(Ebenezer PRIVATE Ebenezer.rc)
endif()

# Enable warnings as errors
if(OPENKO_COMPILE_WARNINGS_AS_ERROR)
  set_property(TARGET Ebenezer PROPERTY COMPILE_WARNING_AS_ERROR ON)
endif()

# Setup PCH
target_precompile_headers(Ebenezer PRIVATE pch.h)

# Setup dependencies
target_link_libraries(Ebenezer PRIVATE
  shared-server
  db-library
  FileIO
)

# Setup warning levels
target_compile_options(Ebenezer PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W3>
  $<$<CXX_COMPILER_ID:GNU>:-Wall>
  $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra>
)

if(MSVC)
  # Override default working directory for ease-of-use
  set_property(TARGET Ebenezer PROPERTY
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:Ebenezer>"
  )
endif()

add_custom_command(TARGET Ebenezer POST_BUILD
  COMMAND "${CMAKE_COMMAND}" -E echo "$<TARGET_FILE:Ebenezer> has been built"
)
