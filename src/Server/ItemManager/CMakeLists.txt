add_library(ItemManager.Core
  Define.h
  ItemManagerApp.cpp
  ItemManagerApp.h
  ItemManagerLogger.cpp
  ItemManagerLogger.h
  ItemManagerReadQueueThread.cpp
  ItemManagerReadQueueThread.h
  pch.h
  resource.h
)

add_executable(ItemManager
  ItemManager.cpp
  pch.h
  resource.h
)

if(WIN32)
    target_sources(ItemManager PRIVATE ItemManager.rc)
endif()

# Enable warnings as errors
if(OPENKO_COMPILE_WARNINGS_AS_ERROR)
  set_property(TARGET ItemManager.Core PROPERTY COMPILE_WARNING_AS_ERROR ON)
  set_property(TARGET ItemManager PROPERTY COMPILE_WARNING_AS_ERROR ON)
endif()

# Setup PCH
target_precompile_headers(ItemManager.Core PRIVATE pch.h)

# Setup dependencies
target_link_libraries(ItemManager.Core PUBLIC
  shared-server
  FileIO
)

target_link_libraries(ItemManager PRIVATE
  ItemManager.Core
)

# Setup warning levels
target_compile_options(ItemManager.Core PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W3>
  $<$<CXX_COMPILER_ID:GNU>:-Wall>
  $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra>
)

target_compile_options(ItemManager PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W3>
  $<$<CXX_COMPILER_ID:GNU>:-Wall>
  $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra>
)

if(MSVC)
  # Override default working directory for ease-of-use
  set_property(TARGET ItemManager PROPERTY
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:ItemManager>"
  )
endif()

add_custom_command(TARGET ItemManager POST_BUILD
  COMMAND "${CMAKE_COMMAND}" -E echo "$<TARGET_FILE:ItemManager> has been built"
)
