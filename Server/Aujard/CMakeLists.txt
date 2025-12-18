add_executable(Aujard
  Aujard.cpp
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

if(WIN32)
    target_sources(Aujard PRIVATE Aujard.rc)
endif()

# Enable warnings as errors
if(OPENKO_COMPILE_WARNINGS_AS_ERROR)
  set_property(TARGET Aujard PROPERTY COMPILE_WARNING_AS_ERROR ON)
endif()

# Setup PCH
target_precompile_headers(Aujard PRIVATE pch.h)

# Setup dependencies
target_link_libraries(Aujard PRIVATE
  shared-server
  db-library
  FileIO
)

# Setup warning levels
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
