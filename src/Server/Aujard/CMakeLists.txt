add_library(Aujard.Core
  AujardApp.cpp
  AujardApp.h
  AujardReadQueueThread.cpp
  AujardReadQueueThread.h
  DBAgent.cpp
  DBAgent.h
  Define.h
  pch.h
  resource.h
)

add_executable(Aujard
  Aujard.cpp
  pch.h
  resource.h
)

set_target_properties(Aujard.Core PROPERTIES
  CXX_SCAN_FOR_MODULES OFF
)

set_target_properties(Aujard PROPERTIES
  CXX_SCAN_FOR_MODULES OFF
)

if(WIN32)
    target_sources(Aujard PRIVATE Aujard.rc)
endif()

# Enable warnings as errors
if(OPENKO_COMPILE_WARNINGS_AS_ERROR)
  set_property(TARGET Aujard.Core PROPERTY COMPILE_WARNING_AS_ERROR ON)
  set_property(TARGET Aujard PROPERTY COMPILE_WARNING_AS_ERROR ON)
endif()

# Setup PCH
target_precompile_headers(Aujard.Core PRIVATE pch.h)

# Setup dependencies
target_link_libraries(Aujard.Core PUBLIC
  FileIO
  shared-server
)

target_link_libraries(Aujard PRIVATE
  Aujard.Core
)

# Expose include path
target_include_directories(Aujard.Core INTERFACE
  "${CMAKE_CURRENT_SOURCE_DIR}/../" # so we can access <Aujard/...>
)

# Setup warning levels
target_compile_options(Aujard.Core PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W3>
  $<$<CXX_COMPILER_ID:GNU>:-Wall>
  $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra>
)

target_compile_options(Aujard PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W3>
  $<$<CXX_COMPILER_ID:GNU>:-Wall>
  $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra>
)

if(MSVC)
  # Override default working directory for ease-of-use
  set_property(TARGET Aujard PROPERTY
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:Aujard>"
  )
endif()

add_custom_command(TARGET Aujard POST_BUILD
  COMMAND "${CMAKE_COMMAND}" -E echo "$<TARGET_FILE:Aujard> has been built"
)
