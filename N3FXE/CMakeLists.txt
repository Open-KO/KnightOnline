find_package(MFC REQUIRED)

# MFC (static)
set(CMAKE_MFC_FLAG 1)

add_executable(N3FXE
  DlgEditFxg.cpp
  DlgEditFxg.h
  DlgEditPartBillBoard.cpp
  DlgEditPartBillBoard.h
  DlgEditPartGround.cpp
  DlgEditPartGround.h
  DlgEditPartMesh.cpp
  DlgEditPartMesh.h
  DlgEditPartParticle.cpp
  DlgEditPartParticle.h
  DlgEditScript.cpp
  DlgEditScript.h
  DlgNewFileName.cpp
  DlgNewFileName.h
  DlgNewPart.cpp
  DlgNewPart.h
  DlgPercent.cpp
  DlgPercent.h
  DlgPutColor.cpp
  DlgPutColor.h
  Ground.cpp
  Ground.h
  MainFrm.cpp
  MainFrm.h
  N3FXE.cpp
  N3FXE.h
  N3FXEDoc.cpp
  N3FXEDoc.h
  N3FXEView.cpp
  N3FXEView.h
  resource.h
  StdAfx.h

  # Resources
  res/Toolbar.bmp)

if(WIN32)
  set_target_properties(N3FXE PROPERTIES WIN32_EXECUTABLE TRUE)
  target_sources(N3FXE PRIVATE
    N3FXE.rc
    res/N3FXE.rc2
    res/N3FXE.ico
    res/N3FXEDoc.ico)
endif()

# Enable warnings as errors
if(OPENKO_COMPILE_WARNINGS_AS_ERROR)
  set_property(TARGET N3FXE PROPERTY COMPILE_WARNING_AS_ERROR ON)
endif()

# Setup PCH
target_precompile_headers(N3FXE PRIVATE StdAfx.h)

# Setup dependencies
target_link_libraries(N3FXE PRIVATE
  N3Base_tool
)

# Setup preprocessor defines
target_compile_definitions(N3FXE PRIVATE
  _CRT_SECURE_NO_WARNINGS)

# Setup warning levels
#target_compile_options(N3FXE PRIVATE
#  $<$<CXX_COMPILER_ID:MSVC>:/W3>
#  $<$<CXX_COMPILER_ID:GNU>:-Wall>
#  $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra>
#)

add_custom_command(TARGET N3FXE POST_BUILD
  COMMAND "${CMAKE_COMMAND}" -E echo "$<TARGET_FILE:N3FXE> has been built"
)
