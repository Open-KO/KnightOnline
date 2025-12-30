find_package(MFC REQUIRED)

# MFC (static)
set(CMAKE_MFC_FLAG 1)

add_executable(N3Viewer
  "../Common Control/PropertyList.cpp"
  "../Common Control/PropertyList.h"
  DlgBrowsePath.cpp
  DlgBrowsePath.h
  DlgPMeshCreateOption.cpp
  DlgPMeshCreateOption.h
  DlgPMeshEdit.cpp
  DlgPMeshEdit.h
  MainFrm.cpp
  MainFrm.h
  N3Viewer.cpp
  N3Viewer.h
  N3ViewerDoc.cpp
  N3ViewerDoc.h
  N3ViewerView.cpp
  N3ViewerView.h
  resource.h
  ShellTree.cpp
  ShellTree.h
  StdAfx.h
  ViewProperty.cpp
  ViewProperty.h
  ViewSceneTree.cpp
  ViewSceneTree.h

  # Resources
  res/NodeIcons.bmp
  res/Toolbar.bmp)

require_directx9(N3Viewer)

if(WIN32)
  set_target_properties(N3Viewer PROPERTIES WIN32_EXECUTABLE TRUE)
  target_sources(N3Viewer PRIVATE
    N3Viewer.rc
    res/N3Viewer.rc2
    res/N3Viewer.ico
    res/N3ViewerDoc.ico)
endif()

# Enable warnings as errors
if(OPENKO_COMPILE_WARNINGS_AS_ERROR)
  set_property(TARGET N3Viewer PROPERTY COMPILE_WARNING_AS_ERROR ON)
endif()

# Setup PCH
target_precompile_headers(N3Viewer PRIVATE StdAfx.h)

# Setup dependencies
target_link_libraries(N3Viewer PRIVATE
  N3Base_tool
)

# Setup preprocessor defines
target_compile_definitions(N3Viewer PRIVATE
  _CRT_SECURE_NO_WARNINGS)

# Setup warning levels
#target_compile_options(N3Viewer PRIVATE
#  $<$<CXX_COMPILER_ID:MSVC>:/W3>
#  $<$<CXX_COMPILER_ID:GNU>:-Wall>
#  $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra>
#)

add_custom_command(TARGET N3Viewer POST_BUILD
  COMMAND "${CMAKE_COMMAND}" -E echo "$<TARGET_FILE:N3Viewer> has been built"
)
