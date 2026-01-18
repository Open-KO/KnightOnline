set(EBENEZER_VS_MAP_DIR    "${OPENKO_SERVER_MAP_SRC_DIR}")
set(EBENEZER_VS_QUESTS_DIR "${OPENKO_SERVER_QUESTS_SRC_DIR}")

add_library(Ebenezer.Core
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

add_executable(Ebenezer
  Ebenezer.cpp
  pch.h
  resource.h
)

set_target_properties(Ebenezer.Core PROPERTIES
  CXX_SCAN_FOR_MODULES OFF
)

set_target_properties(Ebenezer PROPERTIES
  CXX_SCAN_FOR_MODULES OFF
)

# Ensure we require the server assets to be copied
add_dependencies(Ebenezer copy-server-assets)

if(WIN32)
  target_sources(Ebenezer PRIVATE Ebenezer.rc)
endif()

# Enable warnings as errors
if(OPENKO_COMPILE_WARNINGS_AS_ERROR)
  set_property(TARGET Ebenezer.Core PROPERTY COMPILE_WARNING_AS_ERROR ON)
  set_property(TARGET Ebenezer PROPERTY COMPILE_WARNING_AS_ERROR ON)
endif()

# Setup PCH
target_precompile_headers(Ebenezer.Core PRIVATE pch.h)

# Setup dependencies
target_link_libraries(Ebenezer.Core PUBLIC
  FileIO
  shared-server
)

target_link_libraries(Ebenezer PRIVATE
  Ebenezer.Core
)

# Expose include path
target_include_directories(Ebenezer.Core INTERFACE
  "${CMAKE_CURRENT_SOURCE_DIR}/../" # so we can access <Ebenezer/...>
)

# Setup warning levels
target_compile_options(Ebenezer.Core PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W3>
  $<$<CXX_COMPILER_ID:GNU>:-Wall>
  $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra>
)

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

  # Ensure we setup the default MAP & QUESTS dirs as well
  set_property(TARGET Ebenezer PROPERTY
    VS_DEBUGGER_COMMAND_ARGUMENTS "--map-dir=\"${EBENEZER_VS_MAP_DIR}\" --quests-dir=\"${EBENEZER_VS_QUESTS_DIR}\""
  )
endif()

add_custom_command(TARGET Ebenezer POST_BUILD
  COMMAND "${CMAKE_COMMAND}" -E echo "$<TARGET_FILE:Ebenezer> has been built"
)
