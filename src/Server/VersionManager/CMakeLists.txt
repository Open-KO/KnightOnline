add_library(VersionManager.Core
  DBProcess.cpp
  DBProcess.h
  Define.h
  pch.h
  resource.h
  User.cpp
  User.h
  VersionManagerApp.cpp
  VersionManagerApp.h
)

add_executable(VersionManager
  pch.h
  resource.h
  VersionManager.cpp
)

set_target_properties(VersionManager.Core PROPERTIES
  CXX_SCAN_FOR_MODULES OFF
)

set_target_properties(VersionManager PROPERTIES
  CXX_SCAN_FOR_MODULES OFF
)

if(WIN32)
    target_sources(VersionManager PRIVATE VersionManager.rc)
endif()

# Enable warnings as errors
if(OPENKO_COMPILE_WARNINGS_AS_ERROR)
  set_property(TARGET VersionManager.Core PROPERTY COMPILE_WARNING_AS_ERROR ON)
  set_property(TARGET VersionManager PROPERTY COMPILE_WARNING_AS_ERROR ON)
endif()

# Setup PCH
target_precompile_headers(VersionManager.Core PRIVATE pch.h)

# Setup dependencies
target_link_libraries(VersionManager.Core PUBLIC
  shared-server
  FileIO
)

target_link_libraries(VersionManager PRIVATE
  VersionManager.Core
)

# Expose include path
target_include_directories(VersionManager.Core INTERFACE
  "${CMAKE_CURRENT_SOURCE_DIR}/../" # so we can access <VersionManager/...>
)

# Setup warning levels
target_compile_options(VersionManager.Core PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W3>
  $<$<CXX_COMPILER_ID:GNU>:-Wall>
  $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra>
)

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
