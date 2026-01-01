find_package(MFC REQUIRED)

# MFC (static)
set(CMAKE_MFC_FLAG 1)

add_executable(SkyViewer
  "../Common Control/PropertyList.cpp"
  "../Common Control/PropertyList.h"
  FormViewProperty.cpp
  FormViewProperty.h
  MainFrm.cpp
  MainFrm.h
  SkyViewer.cpp
  SkyViewer.h
  SkyViewerDoc.cpp
  SkyViewerDoc.h
  SkyViewerView.cpp
  SkyViewerView.h
  resource.h
  StdAfx.h

  # Resources
  res/Toolbar.bmp)

if(WIN32)
  set_target_properties(SkyViewer PROPERTIES WIN32_EXECUTABLE TRUE)
  target_sources(SkyViewer PRIVATE
    SkyViewer.rc
    res/SkyViewer.rc2
    res/SkyViewer.ico
    res/SkyViewerDoc.ico)
endif()

# Enable warnings as errors
if(OPENKO_COMPILE_WARNINGS_AS_ERROR)
  set_property(TARGET SkyViewer PROPERTY COMPILE_WARNING_AS_ERROR ON)
endif()

# Setup PCH
target_precompile_headers(SkyViewer PRIVATE StdAfx.h)

# Setup dependencies
target_link_libraries(SkyViewer PRIVATE
  N3Base_tool
)

# Setup preprocessor defines
target_compile_definitions(SkyViewer PRIVATE
  _CRT_SECURE_NO_WARNINGS)

# Setup warning levels
#target_compile_options(SkyViewer PRIVATE
#  $<$<CXX_COMPILER_ID:MSVC>:/W3>
#  $<$<CXX_COMPILER_ID:GNU>:-Wall>
#  $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra>
#)

add_custom_command(TARGET SkyViewer POST_BUILD
  COMMAND "${CMAKE_COMMAND}" -E echo "$<TARGET_FILE:SkyViewer> has been built"
)
