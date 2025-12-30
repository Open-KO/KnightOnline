find_package(MFC REQUIRED)

# MFC (static)
set(CMAKE_MFC_FLAG 1)

add_executable(N3TexViewer
  DlgFormat.cpp
  DlgFormat.h
  MainFrm.cpp
  MainFrm.h
  N3TexViewer.cpp
  N3TexViewer.h
  N3TexViewerDoc.cpp
  N3TexViewerDoc.h
  N3TexViewerView.cpp
  N3TexViewerView.h
  ProgressBar.cpp
  ProgressBar.h
  resource.h
  StdAfx.h

  # Resources
  res/Toolbar.bmp)

require_directx9(N3TexViewer)

if(WIN32)
  set_target_properties(N3TexViewer PROPERTIES WIN32_EXECUTABLE TRUE)
  target_sources(N3TexViewer PRIVATE
    N3TexViewer.rc
    res/N3TexViewer.rc2
    res/N3TexViewer.ico
    res/N3TexViewerDoc.ico)
endif()

# Enable warnings as errors
if(OPENKO_COMPILE_WARNINGS_AS_ERROR)
  set_property(TARGET N3TexViewer PROPERTY COMPILE_WARNING_AS_ERROR ON)
endif()

# Setup PCH
target_precompile_headers(N3TexViewer PRIVATE StdAfx.h)

# Setup dependencies
target_link_libraries(N3TexViewer PRIVATE
  N3Base_tool
)

# Setup preprocessor defines
target_compile_definitions(N3TexViewer PRIVATE
  _CRT_SECURE_NO_WARNINGS)

# Setup warning levels
#target_compile_options(N3TexViewer PRIVATE
#  $<$<CXX_COMPILER_ID:MSVC>:/W3>
#  $<$<CXX_COMPILER_ID:GNU>:-Wall>
#  $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra>
#)

add_custom_command(TARGET N3TexViewer POST_BUILD
  COMMAND "${CMAKE_COMMAND}" -E echo "$<TARGET_FILE:N3TexViewer> has been built"
)
