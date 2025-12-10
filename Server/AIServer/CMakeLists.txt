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

# Ensure we require the server assets to be copied
add_dependencies(AIServer copy-server-assets)

if(WIN32)
  target_sources(AIServer PRIVATE AIServer.rc)
endif()

# Enable warnings as errors
if(OPENKO_COMPILE_WARNINGS_AS_ERROR)
  set_property(TARGET AIServer PROPERTY COMPILE_WARNING_AS_ERROR ON)
endif()

# Setup PCH
target_precompile_headers(AIServer PRIVATE pch.h)

# Setup dependencies
target_link_libraries(AIServer PRIVATE
  shared-server
  db-library
)

# Setup warning levels
target_compile_options(AIServer PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W3>
  $<$<CXX_COMPILER_ID:GNU>:-Wall>
  $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra>
)

if(MSVC)
  set_property(TARGET AIServer PROPERTY
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:AIServer>"
  )
endif()

add_custom_command(TARGET AIServer POST_BUILD
  COMMAND "${CMAKE_COMMAND}" -E echo "$<TARGET_FILE:AIServer> has been built"
)
