set(N3BASE_COMMON_AUDIO_SOURCES
  al_wrapper.h
  AudioAsset.cpp
  AudioAsset.h
  AudioDecoderThread.cpp
  AudioDecoderThread.h
  AudioHandle.cpp
  AudioHandle.h
  AudioThread.cpp
  AudioThread.h
  mpg123_reader_io.cpp
  mpg123_reader_io.h
  N3SndDef.h
  N3SndMgr.cpp
  N3SndMgr.h
  N3SndObj.cpp
  N3SndObj.h
)

set(N3BASE_COMMON_EFFECT_SOURCES
  N3FXBundle.cpp
  N3FXBundle.h
  N3FXDef.h
  N3FXPartBase.cpp
  N3FXPartBase.h
  N3FXPartBillBoard.cpp
  N3FXPartBillBoard.h
  N3FXPartBottomBoard.cpp
  N3FXPartBottomBoard.h
  N3FXParticle.cpp
  N3FXParticle.h
  N3FXPartMesh.cpp
  N3FXPartMesh.h
  N3FXPartParticles.cpp
  N3FXPartParticles.h
  N3FXPlug.cpp
  N3FXPlug.h
  N3FXPMesh.cpp
  N3FXPMesh.h
  N3FXPMeshInstance.cpp
  N3FXPMeshInstance.h
  N3FXShape.cpp
  N3FXShape.h
)

set(N3BASE_TOOL_EFFECT_SOURCES
  N3FXGroup.cpp
  N3FXGroup.h
)

set(N3BASE_COMMON_SKY_SOURCES
  N3Cloud.cpp
  N3Cloud.h
  N3Moon.cpp
  N3Moon.h
  N3Sky.cpp
  N3Sky.h
  N3SkyMng.cpp
  N3SkyMng.h
  N3Star.cpp
  N3Star.h
  N3Sun.cpp
  N3Sun.h
)

set(N3BASE_COMMON_UI_SOURCES
  N3UIArea.cpp
  N3UIArea.h
  N3UIBase.cpp
  N3UIBase.h
  N3UIButton.cpp
  N3UIButton.h
  N3UIDef.h
  N3UIEdit.cpp
  N3UIEdit.h
  N3UIImage.cpp
  N3UIImage.h
  N3UIList.cpp
  N3UIList.h
  N3UIProgress.cpp
  N3UIProgress.h
  N3UIScrollBar.cpp
  N3UIScrollBar.h
  N3UIStatic.cpp
  N3UIStatic.h
  N3UIString.cpp
  N3UIString.h
  N3UITooltip.cpp
  N3UITooltip.h
  N3UITrackBar.cpp
  N3UITrackBar.h
)

set(N3BASE_COMMON_WEATHER_SOURCES
  N3GERain.cpp
  N3GERain.h
  N3GESnow.cpp
  N3GESnow.h
  N3GlobalEffect.cpp
  N3GlobalEffect.h
)

set(N3BASE_TOOL_WEATHER_SOURCES
  N3GlobalEffectMng.cpp
  N3GlobalEffectMng.h
)

set(N3BASE_COMMON_SOURCES
  BitMapFile.cpp
  BitMapFile.h
  ColorValue.inl
  DebugUtils.cpp
  DebugUtils.h
  DFont.cpp
  DFont.h
  LogWriter.cpp
  LogWriter.h
  Material.inl
  My_3DStruct.cpp
  My_3DStruct.h
  N3AlphaPrimitiveManager.cpp
  N3AlphaPrimitiveManager.h
  N3AnimControl.cpp
  N3AnimControl.h
  N3AnimKey.cpp
  N3AnimKey.h
  N3Base.cpp
  N3Base.h
  N3BaseFileAccess.cpp
  N3BaseFileAccess.h
  N3Camera.cpp
  N3Camera.h
  N3Chr.cpp
  N3Chr.h
  N3Cloak.cpp
  N3Cloak.h
  N3ColorChange.cpp
  N3ColorChange.h
  N3Eng.cpp
  N3Eng.h
  N3IMesh.cpp
  N3IMesh.h
  N3Joint.cpp
  N3Joint.h
  N3Light.cpp
  N3Light.h
  N3Mesh.cpp
  N3Mesh.h
  N3Mng.h
  N3PMesh.cpp
  N3PMesh.h
  N3PMeshInstance.cpp
  N3PMeshInstance.h
  N3Scene.cpp
  N3Scene.h
  N3Shape.cpp
  N3Shape.h
  N3ShapeExtra.cpp
  N3ShapeExtra.h
  N3ShapeMgr.cpp
  N3ShapeMgr.h
  N3Skin.cpp
  N3Skin.h
  N3TableBase.h
  N3TableBaseImpl.cpp
  N3TableBaseImpl.h
  N3Texture.cpp
  N3Texture.h
  N3Transform.cpp
  N3Transform.h
  N3TransformCollision.cpp
  N3TransformCollision.h
  N3VMesh.cpp
  N3VMesh.h
  StdAfxBase.h
  WaveFormat.cpp
  WaveFormat.h
  WinCrypt.cpp
  WinCrypt.h
  VertexTypes.inl
  ${N3BASE_COMMON_AUDIO_SOURCES}
  ${N3BASE_COMMON_EFFECT_SOURCES}
  ${N3BASE_COMMON_SKY_SOURCES}
  ${N3BASE_COMMON_UI_SOURCES}
  ${N3BASE_COMMON_WEATHER_SOURCES}
)

set(N3BASE_COMMON_DEPENDENCIES_PUBLIC
  dx9sdk
  FileIO
  shared
  spdlog
)

set(N3BASE_COMMON_DEPENDENCIES_PRIVATE
  MathUtils
  libmpg123_wrapper
  OpenAL
)

add_library(N3Base_client STATIC
  ${N3BASE_COMMON_SOURCES}
)

add_library(N3Base_tool STATIC
  N3EngTool.cpp
  N3EngTool.h
  N3PMeshCreate.cpp
  N3PMeshCreate.h
  Pick.cpp
  Pick.h
  ${N3BASE_COMMON_SOURCES}
  ${N3BASE_TOOL_EFFECT_SOURCES}
  ${N3BASE_TOOL_WEATHER_SOURCES}
)

source_group("Audio" FILES ${N3BASE_COMMON_AUDIO_SOURCES})
source_group("Effects" FILES ${N3BASE_COMMON_EFFECT_SOURCES} ${N3BASE_TOOL_EFFECT_SOURCES})
source_group("Sky" FILES ${N3BASE_COMMON_SKY_SOURCES})
source_group("UI" FILES ${N3BASE_COMMON_UI_SOURCES})
source_group("Weather" FILES ${N3BASE_COMMON_WEATHER_SOURCES} ${N3BASE_TOOL_WEATHER_SOURCES})

# Enable warnings as errors
if(OPENKO_COMPILE_WARNINGS_AS_ERROR)
  set_property(TARGET N3Base_client PROPERTY COMPILE_WARNING_AS_ERROR ON)
  set_property(TARGET N3Base_tool   PROPERTY COMPILE_WARNING_AS_ERROR ON)
endif()

# Setup preprocessor defines
target_compile_definitions(N3Base_client PUBLIC
  _N3GAME
  _CRT_SECURE_NO_WARNINGS
  NOMINMAX)

target_compile_definitions(N3Base_tool PUBLIC
  _N3TOOL
  _CRT_SECURE_NO_WARNINGS
  NOMINMAX)

target_compile_definitions(N3Base_tool INTERFACE
  NO_WARN_MBCS_MFC_DEPRECATION)

# Setup dependencies
target_link_libraries(N3Base_client PUBLIC  ${N3BASE_COMMON_DEPENDENCIES_PUBLIC})
target_link_libraries(N3Base_tool   PUBLIC  ${N3BASE_COMMON_DEPENDENCIES_PUBLIC})
target_link_libraries(N3Base_client PRIVATE ${N3BASE_COMMON_DEPENDENCIES_PRIVATE})
target_link_libraries(N3Base_tool   PRIVATE ${N3BASE_COMMON_DEPENDENCIES_PRIVATE})

# Expose include path
target_include_directories(N3Base_client INTERFACE
  "${CMAKE_CURRENT_SOURCE_DIR}/../" # so we can access <N3Base/...>
)
target_include_directories(N3Base_tool INTERFACE
  "${CMAKE_CURRENT_SOURCE_DIR}/../" # so we can access <N3Base/...>
)

# Setup PCH
target_precompile_headers(N3Base_client PRIVATE "StdAfxBase.h")
target_precompile_headers(N3Base_tool   PRIVATE "StdAfxBase.h")

# Setup warning levels
set(N3BASE_COMMON_COMPILE_OPTIONS
  $<$<CXX_COMPILER_ID:MSVC>:/W3>
  $<$<CXX_COMPILER_ID:GNU>:-Wall>
  $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra>
)

target_compile_options(N3Base_client PRIVATE ${N3BASE_COMMON_COMPILE_OPTIONS})
target_compile_options(N3Base_tool   PRIVATE ${N3BASE_COMMON_COMPILE_OPTIONS})
