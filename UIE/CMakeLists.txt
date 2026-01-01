find_package(MFC REQUIRED)

# MFC (static)
set(CMAKE_MFC_FLAG 1)

add_executable(UIE
  DlgAnimate.cpp
  DlgAnimate.h
  DlgBar.cpp
  DlgBar.h
  DlgChangeImage.cpp
  DlgChangeImage.h
  DlgReplace.cpp
  DlgReplace.h
  DlgTexture.cpp
  DlgTexture.h
  DlgUnusedFileList.cpp
  DlgUnusedFileList.h
  HierarchyView.cpp
  HierarchyView.h
  MainFrm.cpp
  MainFrm.h
  PropertyList.cpp
  PropertyList.h
  PropertyView.cpp
  PropertyView.h
  resource.h
  StdAfx.h
  TexViewer.cpp
  TexViewer.h
  UIE.cpp
  UIE.h
  UIEDoc.cpp
  UIEDoc.h
  UIEView.cpp
  UIEView.h

  # Resources
  res/Toolbar.bmp)

if(WIN32)
  set_target_properties(UIE PROPERTIES WIN32_EXECUTABLE TRUE)
  target_sources(UIE PRIVATE
    UIE.rc
    res/UIE.rc2
    res/UIE.ico
    res/UIEDoc.ico
    res/hand.cur
    res/zoomin.cur
    res/zoomout.cur)
endif()

# Enable warnings as errors
if(OPENKO_COMPILE_WARNINGS_AS_ERROR)
  set_property(TARGET UIE PROPERTY COMPILE_WARNING_AS_ERROR ON)
endif()

# Setup PCH
target_precompile_headers(UIE PRIVATE StdAfx.h)

# Setup dependencies
target_link_libraries(UIE PRIVATE
  N3Base_tool
)

# Setup preprocessor defines
target_compile_definitions(UIE PRIVATE
  _CRT_SECURE_NO_WARNINGS)

# Setup warning levels
#target_compile_options(UIE PRIVATE
#  $<$<CXX_COMPILER_ID:MSVC>:/W3>
#  $<$<CXX_COMPILER_ID:GNU>:-Wall>
#  $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra>
#)

add_custom_command(TARGET UIE POST_BUILD
  COMMAND "${CMAKE_COMMAND}" -E echo "$<TARGET_FILE:UIE> has been built"
)
