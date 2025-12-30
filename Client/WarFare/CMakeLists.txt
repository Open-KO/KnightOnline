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

add_executable(WarFare
  APISocket.cpp
  APISocket.h
  Bird.cpp
  Bird.h
  BirdMng.cpp
  BirdMng.h
  Bitset.h
  ClientResourceFormatter.cpp
  ClientResourceFormatter.h
  CountableItemEditDlg.cpp
  CountableItemEditDlg.h
  DungeonManager.cpp
  DungeonManager.h
  EventManager.cpp
  EventManager.h
  GameBase.cpp
  GameBase.h
  GameCursor.cpp
  GameCursor.h
  GameDef.h
  GameEng.cpp
  GameEng.h
  GameProcCharacterCreate.cpp
  GameProcCharacterCreate.h
  GameProcCharacterSelect.cpp
  GameProcCharacterSelect.h
  GameProcedure.cpp
  GameProcedure.h
  GameProcLogIn_1098.cpp
  GameProcLogIn_1098.h
  GameProcLogIn_1298.cpp
  GameProcLogIn_1298.h
  GameProcLogIn.h
  GameProcMain.cpp
  GameProcMain.h
  GameProcNationSelect.cpp
  GameProcNationSelect.h
  GameProcOption.cpp
  GameProcOption.h
  GrassBoard.cpp
  GrassBoard.h
  GrassMng.cpp
  GrassMng.h
  IconItemSkill.cpp
  IconItemSkill.h
  ItemRepairMgr.cpp
  ItemRepairMgr.h
  LightMgr.cpp
  LightMgr.h
  LocalInput.cpp
  LocalInput.h
  MagicSkillMng.cpp
  MagicSkillMng.h
  N3ClientShapeMgr.cpp
  N3ClientShapeMgr.h
  N3FXBundleGame.cpp
  N3FXBundleGame.h
  N3FXMgr.cpp
  N3FXMgr.h
  N3FXPartBillBoardGame.cpp
  N3FXPartBillBoardGame.h
  N3FXPartBottomBoardGame.cpp
  N3FXPartBottomBoardGame.h
  N3Pond.cpp
  N3Pond.h
  N3River.cpp
  N3River.h
  N3Terrain.cpp
  N3Terrain.h
  N3TerrainDef.h
  N3TerrainManager.cpp
  N3TerrainManager.h
  N3TerrainPatch.cpp
  N3TerrainPatch.h
  N3UIDBCLButton.cpp
  N3UIDBCLButton.h
  N3UIIcon.cpp
  N3UIIcon.h
  N3UIWndBase.cpp
  N3UIWndBase.h
  N3WorldBase.cpp
  N3WorldBase.h
  N3WorldManager.cpp
  N3WorldManager.h
  OrderMessage.cpp
  OrderMessage.h
  PacketDef.h
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
  PortalVolume.cpp
  PortalVolume.h
  PvsMgr.cpp
  PvsMgr.h
  Resource.rc
  ServerMesh.cpp
  ServerMesh.h
  StdAfx.h
  SubProcPerTrade.cpp
  SubProcPerTrade.h
  UICharacterCreate.cpp
  UICharacterCreate.h
  UICharacterSelect.cpp
  UICharacterSelect.h
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
  UILogin_1098.cpp
  UILogin_1098.h
  UILogin_1298.cpp
  UILogin_1298.h
  UIManager.cpp
  UIManager.h
  UIMessageBox.cpp
  UIMessageBox.h
  UIMessageBoxManager.cpp
  UIMessageBoxManager.h
  UIMessageWnd.cpp
  UIMessageWnd.h
  UIMsgBoxOkCancel.cpp
  UIMsgBoxOkCancel.h
  UINationSelectDlg.cpp
  UINationSelectDlg.h
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
  WarFareMain.cpp
  WarMessage.cpp
  WarMessage.h
  "${TARGET_CLIENT_CONFIG_HEADER}"
)

require_directx9(WarFare)

set_target_properties(WarFare PROPERTIES
  OUTPUT_NAME "KnightOnLine"
)

if(WIN32)
  set_target_properties(WarFare PROPERTIES WIN32_EXECUTABLE TRUE)
  target_sources(WarFare PRIVATE Resource.rc)
endif()

# Enable warnings as errors
if(OPENKO_COMPILE_WARNINGS_AS_ERROR)
  set_property(TARGET WarFare PROPERTY COMPILE_WARNING_AS_ERROR ON)
endif()

# Setup PCH
target_precompile_headers(WarFare PRIVATE StdAfx.h)

# Setup dependencies
target_link_libraries(WarFare PRIVATE
  FileIO
  JpegFile
  N3Base_client
)

# Link in the libs that we use
if(WIN32)
  target_link_libraries(WarFare PRIVATE
    imm32.lib
    winmm.lib
    ws2_32.lib
    wsock32.lib
  )
endif()

# Setup preprocessor defines
target_compile_definitions(WarFare PRIVATE
  _WINSOCK_DEPRECATED_NO_WARNINGS)

# Setup warning levels
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
