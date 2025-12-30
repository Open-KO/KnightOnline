find_package(MFC REQUIRED)

# MFC (static)
set(CMAKE_MFC_FLAG 1)

add_executable(N3ME
  "../Common Control/PropertyList.cpp"
  "../Common Control/PropertyList.h"
  BrushDlg.cpp
  BrushDlg.h
  DlgAddDTex.cpp
  DlgAddDTex.h
  DlgAddSoundGroup.cpp
  DlgAddSoundGroup.h
  DlgBar.cpp
  DlgBar.h
  DlgBase.cpp
  DlgBase.h
  DlgBrowsePath.cpp
  DlgBrowsePath.h
  DlgCtrlHeightScale.cpp
  DlgCtrlHeightScale.h
  DlgDelGroup.cpp
  DlgDelGroup.h
  DlgDTexGroupView.cpp
  DlgDTexGroupView.h
  DlgEditEvent.cpp
  DlgEditEvent.h
  DlgEditEventAttr.cpp
  DlgEditEventAttr.h
  DlgEditWarp.cpp
  DlgEditWarp.h
  DlgInputAttr.cpp
  DlgInputAttr.h
  DlgInputGroup.cpp
  DlgInputGroup.h
  DlgLight.cpp
  DlgLight.h
  DlgLoadEvt.cpp
  DlgLoadEvt.h
  DlgLoadNPCPath.cpp
  DlgLoadNPCPath.h
  DlgLoadTileSet.cpp
  DlgLoadTileSet.h
  DlgMakeNPCPath.cpp
  DlgMakeNPCPath.h
  DlgMakeWall.cpp
  DlgMakeWall.h
  DlgMapView.cpp
  DlgMapView.h
  DlgModifyDTex.cpp
  DlgModifyDTex.h
  DlgPondProperty.cpp
  DlgPondProperty.h
  DlgRegenUser.cpp
  DlgRegenUser.h
  DlgRiverProperty.cpp
  DlgRiverProperty.h
  DlgSaveDivision.cpp
  DlgSaveDivision.h
  DlgSaveEvt.cpp
  DlgSaveEvt.h
  DlgSaveNewTileSet.cpp
  DlgSaveNewTileSet.h
  DlgSaveNPCPath.cpp
  DlgSaveNPCPath.h
  DlgSceneGraph.cpp
  DlgSceneGraph.h
  DlgSetDTex.cpp
  DlgSetDTex.h
  DlgSetLightMap.cpp
  DlgSetLightMap.h
  DlgSetSound.cpp
  DlgSetSound.h
  DlgShapeList.cpp
  DlgShapeList.h
  DlgSowSeed.cpp
  DlgSowSeed.h
  DlgTerrainSize.cpp
  DlgTerrainSize.h
  DlgUnusedFiles.cpp
  DlgUnusedFiles.h
  DTex.cpp
  DTex.h
  DTexGroup.cpp
  DTexGroup.h
  DTexGroupMng.cpp
  DTexGroupMng.h
  DTexMng.cpp
  DTexMng.h
  EventCell.cpp
  EventCell.h
  EventMgr.cpp
  EventMgr.h
  LightObjMgr.cpp
  LightObjMgr.h
  LyTerrain.cpp
  LyTerrain.h
  LyTerrainDef.h
  MainFrm.cpp
  MainFrm.h
  MapMng.cpp
  MapMng.h
  N3ME.cpp
  N3ME.h
  N3MEDoc.cpp
  N3MEDoc.h
  N3MEView.cpp
  N3MEView.h
  NPCPath.cpp
  NPCPath.h
  NPCPathMgr.cpp
  NPCPathMgr.h
  PondMesh.cpp
  PondMesh.h
  PondMng.cpp
  PondMng.h
  PosDummy.cpp
  PosDummy.h
  ProgressBar.cpp
  ProgressBar.h
  QTNode.cpp
  QTNode.h
  RegenUser.cpp
  RegenUser.h
  Resource.h
  RiverMesh.cpp
  RiverMesh.h
  RiverMng.cpp
  RiverMng.h
  RotDummy.cpp
  RotDummy.h
  ScaleDummy.cpp
  ScaleDummy.h
  ShellTree.cpp
  ShellTree.h
  SoundCell.cpp
  SoundCell.h
  SoundMgr.cpp
  SoundMgr.h
  SowSeedMng.cpp
  SowSeedMng.h
  StdAfx.h
  TransDummy.cpp
  TransDummy.h
  VtxPosDummy.cpp
  VtxPosDummy.h
  Wall.cpp
  Wall.h
  WallMgr.cpp
  WallMgr.h
  WarpMgr.cpp
  WarpMgr.h)

require_directx9(N3ME)

if(WIN32)
  set_target_properties(N3ME PROPERTIES WIN32_EXECUTABLE TRUE)
  target_sources(N3ME PRIVATE
    N3ME.rc
    res/N3ME.rc2
    res/N3ME.ico
    res/N3MEDoc.ico
    res/load.cur
    res/move.cur
    res/rotate.cur
    res/transform.cur
    res/zoom.cur)
endif()

# Enable warnings as errors
if(OPENKO_COMPILE_WARNINGS_AS_ERROR)
  set_property(TARGET N3ME PROPERTY COMPILE_WARNING_AS_ERROR ON)
endif()

# Setup PCH
target_precompile_headers(N3ME PRIVATE StdAfx.h)

# Setup dependencies
target_link_libraries(N3ME PRIVATE
  N3Base_tool
)

# Setup preprocessor defines
target_compile_definitions(N3ME PRIVATE
  _KNIGHT
  _CRT_SECURE_NO_WARNINGS)

# Setup warning levels
#target_compile_options(N3ME PRIVATE
#  $<$<CXX_COMPILER_ID:MSVC>:/W3>
#  $<$<CXX_COMPILER_ID:GNU>:-Wall>
#  $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra>
#)

add_custom_command(TARGET N3ME POST_BUILD
  COMMAND "${CMAKE_COMMAND}" -E echo "$<TARGET_FILE:N3ME> has been built"
)
