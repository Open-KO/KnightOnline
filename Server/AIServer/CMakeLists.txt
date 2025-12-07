add_executable(AIServer
  AIServer.cpp
  AIServerApp.cpp
  AIServerApp.h
  AIServerLogger.cpp
  AIServerLogger.h
  AISocketManager.cpp
  AISocketManager.h
  Define.h
  Extern.h
  GameSocket.cpp
  GameSocket.h
  MagicProcess.cpp
  MagicProcess.h
  Map.cpp
  Map.h
  Npc.cpp
  Npc.h
  NpcItem.cpp
  NpcItem.h
  NpcMagicProcess.cpp
  NpcMagicProcess.h
  NpcThread.cpp
  NpcThread.h
  Packet.h
  Party.cpp
  Party.h
  PathFind.cpp
  PathFind.h
  pch.h
  Region.cpp
  Region.h
  resource.h
  RoomEvent.cpp
  RoomEvent.h
  SendThreadMain.cpp
  SendThreadMain.h
  User.cpp
  User.h
  ZoneEventThread.cpp
  ZoneEventThread.h
)

if(WIN32)
    target_sources(AIServer PRIVATE AIServer.rc)
endif()

# Use C++23
target_compile_features(AIServer PUBLIC cxx_std_23)

# Setup PCH
target_precompile_headers(AIServer PRIVATE pch.h)

# Setup dependencies
target_link_libraries(AIServer
  PRIVATE
    shared-server
    db-library
)
