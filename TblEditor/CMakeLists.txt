find_package(MFC REQUIRED)

# MFC (static)
set(CMAKE_MFC_FLAG 1)

add_executable(TblEditor
  framework.h
  targetver.h
  TblEditor.cpp
  TblEditor.h
  TblEditorBase.cpp
  TblEditorBase.h
  TblEditorDoc.cpp
  TblEditorDoc.h
  TblEditorView.cpp
  TblEditorView.h
  TblListCtrl.cpp
  TblListCtrl.h
  resource.h)

if(WIN32)
  set_target_properties(TblEditor PROPERTIES WIN32_EXECUTABLE TRUE)
  target_sources(TblEditor PRIVATE
    TblEditor.rc
    res/TblEditor.rc2
    res/TblEditor.ico)
  target_compile_definitions(TblEditor PRIVATE
    "_UNICODE"
    "UNICODE"
  )
endif()

# Enable warnings as errors
if(OPENKO_COMPILE_WARNINGS_AS_ERROR)
  set_property(TARGET TblEditor PROPERTY COMPILE_WARNING_AS_ERROR ON)
endif()

# Setup PCH
target_precompile_headers(TblEditor PRIVATE StdAfx.h)

# Setup dependencies
target_link_libraries(TblEditor PRIVATE
  N3Base_tool
)

# Setup preprocessor defines
target_compile_definitions(TblEditor PRIVATE
  _CRT_SECURE_NO_WARNINGS)

# Setup warning levels
#target_compile_options(TblEditor PRIVATE
#  $<$<CXX_COMPILER_ID:MSVC>:/W3>
#  $<$<CXX_COMPILER_ID:GNU>:-Wall>
#  $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra>
#)

add_custom_command(TARGET TblEditor POST_BUILD
  COMMAND "${CMAKE_COMMAND}" -E echo "$<TARGET_FILE:TblEditor> has been built"
)
