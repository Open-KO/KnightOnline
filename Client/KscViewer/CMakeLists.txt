find_package(MFC REQUIRED)

# MFC (static)
set(CMAKE_MFC_FLAG 1)

add_executable(KscViewer
  KscViewer.cpp
  KscViewer.h
  KscViewerDoc.cpp
  KscViewerDoc.h
  KscViewerView.cpp
  KscViewerView.h
  MainFrm.cpp
  MainFrm.h
  N3JpegFile.cpp
  N3JpegFile.h
  Resource.h
  StdAfx.h)

if(WIN32)
  set_target_properties(KscViewer PROPERTIES WIN32_EXECUTABLE TRUE)
  target_sources(KscViewer PRIVATE KscViewer.rc)
  target_compile_definitions(KscViewer PRIVATE
    "_UNICODE"
    "UNICODE"
  )
endif()

# Enable warnings as errors
if(OPENKO_COMPILE_WARNINGS_AS_ERROR)
  set_property(TARGET KscViewer PROPERTY COMPILE_WARNING_AS_ERROR ON)
endif()

# Setup PCH
target_precompile_headers(KscViewer PRIVATE StdAfx.h)

# Setup dependencies
target_link_libraries(KscViewer PRIVATE
  JpegFile
)

# Setup warning levels
target_compile_options(KscViewer PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W3>
  $<$<CXX_COMPILER_ID:GNU>:-Wall>
  $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra>
)

add_custom_command(TARGET KscViewer POST_BUILD
  COMMAND "${CMAKE_COMMAND}" -E echo "$<TARGET_FILE:KscViewer> has been built"
)
