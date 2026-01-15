# We use a user-modifiable config file for the client named warfare_config.h
# This is copied from the default if it doesn't already exist.
set(DEFAULT_CLIENT_CONFIG_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/warfare_config.h.default")
set(TARGET_CLIENT_CONFIG_DIR "${CMAKE_CURRENT_BINARY_DIR}/WarFare")
set(TARGET_CLIENT_CONFIG_HEADER "${TARGET_CLIENT_CONFIG_DIR}/warfare_config.h")

# Make sure the target directory exists
file(MAKE_DIRECTORY "${TARGET_CLIENT_CONFIG_DIR}")

if(NOT EXISTS "${TARGET_CLIENT_CONFIG_HEADER}")
  configure_file("${DEFAULT_CLIENT_CONFIG_HEADER}" "${TARGET_CLIENT_CONFIG_HEADER}" COPYONLY)
endif()

set(WARFARE_SCENE_SOURCES
  GameProcedure.cpp
  GameProcedure.h
)

set(WARFARE_SCENE_LOGIN_SOURCES
  GameProcLogIn_1098.cpp
  GameProcLogIn_1098.h
  GameProcLogIn_1298.cpp
  GameProcLogIn_1298.h
  GameProcLogIn.h
  UILogin_1098.cpp
  UILogin_1098.h
  UILogin_1298.cpp
  UILogin_1298.h
)

set(WARFARE_SCENE_NATION_SELECTION_SOURCES
  GameProcNationSelect.cpp
  GameProcNationSelect.h
  UINationSelectDlg.cpp
  UINationSelectDlg.h
)

set(WARFARE_SCENE_CHARACTER_SELECTION_SOURCES
  GameProcCharacterSelect.cpp
  GameProcCharacterSelect.h
  UICharacterSelect.cpp
  UICharacterSelect.h
)

set(WARFARE_SCENE_CHARACTER_CREATION_SOURCES
  GameProcCharacterCreate.cpp
  GameProcCharacterCreate.h
  UICharacterCreate.cpp
  UICharacterCreate.h
)

set(WARFARE_SCENE_INGAME_SOURCES
  GameProcMain.cpp
  GameProcMain.h
)

set(WARFARE_SCENE_INGAME_UI_SOURCES
  ItemRepairMgr.cpp
  ItemRepairMgr.h
  SubProcPerTrade.cpp
  SubProcPerTrade.h
  UIChat.cpp
  UIChat.h
  UIClassChange.cpp
  UIClassChange.h
  UICmd.cpp
  UICmd.h
  UICmdEdit.cpp
  UICmdEdit.h
  UICmdList.cpp
  UICmdList.h
  UICreateClanName.cpp
  UICreateClanName.h
  UIDead.cpp
  UIDead.h
  UIDroppedItemDlg.cpp
  UIDroppedItemDlg.h
  UIEndingDisplay.cpp
  UIEndingDisplay.h
  UIExitMenu.cpp
  UIExitMenu.h
  UIHelp.cpp
  UIHelp.h
  UIHotKeyDlg.cpp
  UIHotKeyDlg.h
  UIImageTooltipDlg.cpp
  UIImageTooltipDlg.h
  UIInn.cpp
  UIInn.h
  UIInventory.cpp
  UIInventory.h
  UIKnightsOperation.cpp
  UIKnightsOperation.h
  UILevelGuide.cpp
  UILevelGuide.h
  UILoading.cpp
  UILoading.h
  UIMessageWnd.cpp
  UIMessageWnd.h
  UIMsgBoxOkCancel.cpp
  UIMsgBoxOkCancel.h
  UINotice.cpp
  UINotice.h
  UINPCChangeEvent.cpp
  UINPCChangeEvent.h
  UINPCEvent.cpp
  UINPCEvent.h
  UIPartyBBS.cpp
  UIPartyBBS.h
  UIPartyBBSSelector.cpp
  UIPartyBBSSelector.h
  UIPartyOrForce.cpp
  UIPartyOrForce.h
  UIPerTradeDlg.cpp
  UIPerTradeDlg.h
  UIPointInitDlg.cpp
  UIPointInitDlg.h
  UIQuestMenu.cpp
  UIQuestMenu.h
  UIQuestTalk.cpp
  UIQuestTalk.h
  UIRepairTooltipDlg.cpp
  UIRepairTooltipDlg.h
  UISkillTreeDlg.cpp
  UISkillTreeDlg.h
  UIStateBar.cpp
  UIStateBar.h
  UITargetBar.cpp
  UITargetBar.h
  UITradeBBSEditDlg.cpp
  UITradeBBSEditDlg.h
  UITradeBBSSelector.cpp
  UITradeBBSSelector.h
  UITradeEditDlg.cpp
  UITradeEditDlg.h
  UITradeExplanation.cpp
  UITradeExplanation.h
  UITradeSellBBS.cpp
  UITradeSellBBS.h
  UITransactionDlg.cpp
  UITransactionDlg.h
  UIUpgradeSelect.cpp
  UIUpgradeSelect.h
  UIVarious.cpp
  UIVarious.h
  UIWareHouseDlg.cpp
  UIWareHouseDlg.h
  UIWarp.cpp
  UIWarp.h
)

set(WARFARE_NETWORK_SOURCES
  APISocket.cpp
  APISocket.h
  PacketDef.h
)

set(WARFARE_INPUT_SOURCES
  LocalInput.cpp
  LocalInput.h
)

set(WARFARE_PLAYERS_SOURCES
  MagicSkillMng.cpp
  MagicSkillMng.h
  PlayerBase.cpp
  PlayerBase.h
  PlayerMySelf.cpp
  PlayerMySelf.h
  PlayerNPC.cpp
  PlayerNPC.h
  PlayerOther.cpp
  PlayerOther.h
  PlayerOtherMgr.cpp
  PlayerOtherMgr.h
)

set(WARFARE_WORLD_SOURCES
  EventManager.cpp
  EventManager.h
  N3WorldBase.cpp
  N3WorldBase.h
  N3WorldManager.cpp
  N3WorldManager.h
  ServerMesh.cpp
  ServerMesh.h
)

set(WARFARE_WORLD_BIRDS_SOURCES
  Bird.cpp
  Bird.h
  BirdMng.cpp
  BirdMng.h
)

set(WARFARE_WORLD_EFFECTS_SOURCES
  N3FXBundleGame.cpp
  N3FXBundleGame.h
  N3FXMgr.cpp
  N3FXMgr.h
  N3FXPartBillBoardGame.cpp
  N3FXPartBillBoardGame.h
  N3FXPartBottomBoardGame.cpp
  N3FXPartBottomBoardGame.h
)

set(WARFARE_WORLD_INDOOR_SOURCES
  DungeonManager.cpp
  DungeonManager.h
  PortalVolume.cpp
  PortalVolume.h
  PvsMgr.cpp
  PvsMgr.h
)

set(WARFARE_WORLD_LIGHTING_SOURCES
  LightMgr.cpp
  LightMgr.h
)

set(WARFARE_WORLD_OBJECTS_SOURCES
  N3ClientShapeMgr.cpp
  N3ClientShapeMgr.h
)

set(WARFARE_WORLD_TERRAIN_SOURCES
  N3Terrain.cpp
  N3Terrain.h
  N3TerrainDef.h
  N3TerrainManager.cpp
  N3TerrainManager.h
  N3TerrainPatch.cpp
  N3TerrainPatch.h
)

set(WARFARE_WORLD_WATER_SOURCES
  N3Pond.cpp
  N3Pond.h
  N3River.cpp
  N3River.h
)

set(WARFARE_CORE_UIS_SOURCES
  CountableItemEditDlg.cpp
  CountableItemEditDlg.h
  GameCursor.cpp
  GameCursor.h
  IconItemSkill.cpp
  IconItemSkill.h
  N3UIDBCLButton.cpp
  N3UIDBCLButton.h
  N3UIIcon.cpp
  N3UIIcon.h
  N3UIWndBase.cpp
  N3UIWndBase.h
  UIManager.cpp
  UIManager.h
  UIMessageBox.cpp
  UIMessageBox.h
  UIMessageBoxManager.cpp
  UIMessageBoxManager.h
)

set(WARFARE_POPUP_NOTICE_TEXT_SOURCES
  OrderMessage.cpp
  OrderMessage.h
  WarMessage.cpp
  WarMessage.h
)

add_library(WarFare.Core
  Bitset.h
  ClientResourceFormatter.cpp
  ClientResourceFormatter.h
  GameBase.cpp
  GameBase.h
  GameDef.h
  GameEng.cpp
  GameEng.h
  StdAfx.h
  WarFareMain.cpp
  ${WARFARE_SCENE_SOURCES}
  ${WARFARE_SCENE_LOGIN_SOURCES}
  ${WARFARE_SCENE_NATION_SELECTION_SOURCES}
  ${WARFARE_SCENE_CHARACTER_SELECTION_SOURCES}
  ${WARFARE_SCENE_CHARACTER_CREATION_SOURCES}
  ${WARFARE_SCENE_INGAME_SOURCES}
  ${WARFARE_SCENE_INGAME_UI_SOURCES}
  ${WARFARE_INPUT_SOURCES}
  ${WARFARE_NETWORK_SOURCES}
  ${WARFARE_PLAYERS_SOURCES}
  ${WARFARE_WORLD_SOURCES}
  ${WARFARE_WORLD_BIRDS_SOURCES}
  ${WARFARE_WORLD_EFFECTS_SOURCES}
  ${WARFARE_WORLD_INDOOR_SOURCES}
  ${WARFARE_WORLD_LIGHTING_SOURCES}
  ${WARFARE_WORLD_OBJECTS_SOURCES}
  ${WARFARE_WORLD_TERRAIN_SOURCES}
  ${WARFARE_WORLD_WATER_SOURCES}
  ${WARFARE_CORE_UIS_SOURCES}
  ${WARFARE_POPUP_NOTICE_TEXT_SOURCES}
  "${TARGET_CLIENT_CONFIG_HEADER}"
)

add_executable(WarFare
  resource.h
  StdAfx.h
  WarFareMain.cpp
)

source_group("Scene" FILES ${WARFARE_SCENE_SOURCES})
source_group("Scene/Login" FILES ${WARFARE_SCENE_LOGIN_SOURCES})
source_group("Scene/Nation selection" FILES ${WARFARE_SCENE_NATION_SELECTION_SOURCES})
source_group("Scene/Character selection" FILES ${WARFARE_SCENE_CHARACTER_SELECTION_SOURCES})
source_group("Scene/Character creation" FILES ${WARFARE_SCENE_CHARACTER_CREATION_SOURCES})
source_group("Scene/Ingame" FILES ${WARFARE_SCENE_INGAME_SOURCES})
source_group("Scene/Ingame/UI" FILES ${WARFARE_SCENE_INGAME_UI_SOURCES})
source_group("Input" FILES ${WARFARE_INPUT_SOURCES})
source_group("Network" FILES ${WARFARE_NETWORK_SOURCES})
source_group("Players" FILES ${WARFARE_PLAYERS_SOURCES})
source_group("World" FILES ${WARFARE_WORLD_SOURCES})
source_group("World/Birds" FILES ${WARFARE_WORLD_BIRDS_SOURCES})
source_group("World/Effects" FILES ${WARFARE_WORLD_EFFECTS_SOURCES})
source_group("World/Indoor" FILES ${WARFARE_WORLD_INDOOR_SOURCES})
source_group("World/Lighting" FILES ${WARFARE_WORLD_LIGHTING_SOURCES})
source_group("World/Objects" FILES ${WARFARE_WORLD_EFFECTS_SOURCES})
source_group("World/Terrain" FILES ${WARFARE_WORLD_TERRAIN_SOURCES})
source_group("World/Water" FILES ${WARFARE_WORLD_WATER_SOURCES})
source_group("Core UIs" FILES ${WARFARE_CORE_UIS_SOURCES})
source_group("Popup notice text" FILES ${WARFARE_POPUP_NOTICE_TEXT_SOURCES})

set_target_properties(WarFare.Core PROPERTIES
  CXX_SCAN_FOR_MODULES OFF
)

set_target_properties(WarFare PROPERTIES
  OUTPUT_NAME          "KnightOnLine"
  CXX_SCAN_FOR_MODULES OFF
)

if(WIN32)
  set_target_properties(WarFare PROPERTIES WIN32_EXECUTABLE TRUE)
  target_sources(WarFare PRIVATE Resource.rc)
endif()

# Enable warnings as errors
if(OPENKO_COMPILE_WARNINGS_AS_ERROR)
  set_property(TARGET WarFare.Core PROPERTY COMPILE_WARNING_AS_ERROR ON)
  set_property(TARGET WarFare PROPERTY COMPILE_WARNING_AS_ERROR ON)
endif()

# Setup PCH
target_precompile_headers(WarFare.Core PRIVATE StdAfx.h)

# Expose include path
target_include_directories(WarFare.Core PUBLIC
  "${TARGET_CLIENT_CONFIG_DIR}/"    # we copied warfare_config.h here, so we should be able to access it too
)

# Setup dependencies
target_link_libraries(WarFare.Core PUBLIC
  FileIO
  JpegFile
  N3Base_client
)

target_link_libraries(WarFare PRIVATE
  WarFare.Core
)

# Link in the libs that we use
if(WIN32)
  target_link_libraries(WarFare.Core PRIVATE
    imm32.lib
    winmm.lib
    ws2_32.lib
    wsock32.lib
  )
endif()

# Setup preprocessor defines
target_compile_definitions(WarFare.Core PRIVATE
  _WINSOCK_DEPRECATED_NO_WARNINGS)

target_compile_definitions(WarFare PRIVATE
  _WINSOCK_DEPRECATED_NO_WARNINGS)

# Setup warning levels
target_compile_options(WarFare.Core PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W3>
  $<$<CXX_COMPILER_ID:GNU>:-Wall>
  $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra>
)

target_compile_options(WarFare PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W3>
  $<$<CXX_COMPILER_ID:GNU>:-Wall>
  $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra>
)

if(MSVC)
  set_property(TARGET WarFare PROPERTY
    VS_DEBUGGER_WORKING_DIRECTORY "${OPENKO_CLIENT_DIR}"
  )
endif()

add_custom_command(TARGET WarFare POST_BUILD
  COMMAND "${CMAKE_COMMAND}" -E echo "$<TARGET_FILE:WarFare> has been built"
)
