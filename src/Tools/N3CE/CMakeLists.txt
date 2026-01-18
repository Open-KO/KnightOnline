find_package(MFC REQUIRED)

# MFC (static)
set(CMAKE_MFC_FLAG 1)

add_executable(N3CE
  "../Common Control/PropertyList.cpp"
  "../Common Control/PropertyList.h"
  FormViewAnimation.cpp
  FormViewAnimation.h
  FormViewProperty.cpp
  FormViewProperty.h
  FormViewTool.cpp
  FormViewTool.h
  MacSliderCtrl.cpp
  MacSliderCtrl.h
  MainFrm.cpp
  MainFrm.h
  N3CE.cpp
  N3CE.h
  N3CEDoc.cpp
  N3CEDoc.h
  N3CEView.cpp
  N3CEView.h
  PosDummy.cpp
  PosDummy.h
  resource.h
  StdAfx.h
  TransDummy.cpp
  TransDummy.h

  # Resources
  res/NodeIcons.bmp
  res/Toolbar.bmp)

set_target_properties(N3CE PROPERTIES
  CXX_SCAN_FOR_MODULES OFF
)

if(WIN32)
  set_target_properties(N3CE PROPERTIES WIN32_EXECUTABLE TRUE)
  target_sources(N3CE PRIVATE
    N3CE.rc
    res/N3CE.rc2
    res/N3CE.ico
    res/N3CEDoc.ico)
endif()

# Enable warnings as errors
if(OPENKO_COMPILE_WARNINGS_AS_ERROR)
  set_property(TARGET N3CE PROPERTY COMPILE_WARNING_AS_ERROR ON)
endif()

# Setup PCH
target_precompile_headers(N3CE PRIVATE StdAfx.h)

# Setup dependencies
target_link_libraries(N3CE PRIVATE
  N3Base_tool
)

# Setup preprocessor defines
target_compile_definitions(N3CE PRIVATE
  _CRT_SECURE_NO_WARNINGS)

# Setup warning levels
#target_compile_options(N3CE PRIVATE
#  $<$<CXX_COMPILER_ID:MSVC>:/W3>
#  $<$<CXX_COMPILER_ID:GNU>:-Wall>
#  $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra>
#)

add_custom_command(TARGET N3CE POST_BUILD
  COMMAND "${CMAKE_COMMAND}" -E echo "$<TARGET_FILE:N3CE> has been built"
)
