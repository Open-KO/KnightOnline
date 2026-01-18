set(AISERVER_VS_MAP_DIR   "${OPENKO_SERVER_MAP_SRC_DIR}")
set(AISERVER_VS_EVENT_DIR "${OPENKO_SERVER_MAP_SRC_DIR}") # Intentionally the same

add_library(AIServer.Core
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

add_executable(AIServer
  AIServer.cpp
  pch.h
  resource.h
)

set_target_properties(AIServer.Core PROPERTIES
  CXX_SCAN_FOR_MODULES OFF
)

set_target_properties(AIServer PROPERTIES
  CXX_SCAN_FOR_MODULES OFF
)

# Ensure we require the server assets to be copied
add_dependencies(AIServer copy-server-assets)

if(WIN32)
  target_sources(AIServer PRIVATE AIServer.rc)
endif()

# Enable warnings as errors
if(OPENKO_COMPILE_WARNINGS_AS_ERROR)
  set_property(TARGET AIServer.Core PROPERTY COMPILE_WARNING_AS_ERROR ON)
  set_property(TARGET AIServer PROPERTY COMPILE_WARNING_AS_ERROR ON)
endif()

# Setup PCH
target_precompile_headers(AIServer.Core PRIVATE pch.h)

# Setup dependencies
target_link_libraries(AIServer.Core PUBLIC
  FileIO
  shared-server
)

target_link_libraries(AIServer PRIVATE
  AIServer.Core
)

# Expose include path
target_include_directories(AIServer.Core INTERFACE
  "${CMAKE_CURRENT_SOURCE_DIR}/../" # so we can access <AIServer/...>
)

# Setup warning levels
target_compile_options(AIServer.Core PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W3>
  $<$<CXX_COMPILER_ID:GNU>:-Wall>
  $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra>
)

target_compile_options(AIServer PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W3>
  $<$<CXX_COMPILER_ID:GNU>:-Wall>
  $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra>
)

if(MSVC)
  # Override default working directory for ease-of-use
  set_property(TARGET AIServer PROPERTY
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:AIServer>"
  )

  # Ensure we setup the default MAP & QUESTS dirs as well
  set_property(TARGET AIServer PROPERTY
    VS_DEBUGGER_COMMAND_ARGUMENTS "--map-dir=\"${AISERVER_VS_MAP_DIR}\" --event-dir=\"${AISERVER_VS_EVENT_DIR}\""
  )
endif()

add_custom_command(TARGET AIServer POST_BUILD
  COMMAND "${CMAKE_COMMAND}" -E echo "$<TARGET_FILE:AIServer> has been built"
)
