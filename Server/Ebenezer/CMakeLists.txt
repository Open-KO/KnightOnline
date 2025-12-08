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

if(WIN32)
    target_sources(Ebenezer PRIVATE Ebenezer.rc)
endif()

# Use C++23
target_compile_features(Ebenezer PUBLIC cxx_std_23)

# Enable warnings as errors
if(OPENKO_COMPILE_WARNINGS_AS_ERROR)
  set_property(TARGET Ebenezer PROPERTY COMPILE_WARNING_AS_ERROR ON)
endif()

# Setup PCH
target_precompile_headers(Ebenezer PRIVATE pch.h)

# Setup dependencies
target_link_libraries(Ebenezer
  PRIVATE
    shared-server
    db-library
)

# Setup warning levels
target_compile_options(Ebenezer PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W3>
  $<$<CXX_COMPILER_ID:GNU>:-Wall>
  $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra>
)
