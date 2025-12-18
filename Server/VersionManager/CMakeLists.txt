add_executable(VersionManager
  DBProcess.cpp
  DBProcess.h
  Define.h
  pch.h
  resource.h
  User.cpp
  User.h
  VersionManager.cpp
  VersionManagerApp.cpp
  VersionManagerApp.h
)

if(WIN32)
    target_sources(VersionManager PRIVATE VersionManager.rc)
endif()

# Enable warnings as errors
if(OPENKO_COMPILE_WARNINGS_AS_ERROR)
  set_property(TARGET VersionManager PROPERTY COMPILE_WARNING_AS_ERROR ON)
endif()

# Setup PCH
target_precompile_headers(VersionManager PRIVATE pch.h)

# Setup dependencies
target_link_libraries(VersionManager PRIVATE
  shared-server
  db-library
  FileIO
)

# Setup warning levels
target_compile_options(VersionManager PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W3>
  $<$<CXX_COMPILER_ID:GNU>:-Wall>
  $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra>
)

if(MSVC)
  # Override default working directory for ease-of-use
  set_property(TARGET VersionManager PROPERTY
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:VersionManager>"
  )
endif()

add_custom_command(TARGET VersionManager POST_BUILD
  COMMAND "${CMAKE_COMMAND}" -E echo "$<TARGET_FILE:VersionManager> has been built"
)
