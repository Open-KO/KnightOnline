add_executable(ItemManager
  Define.h
  ItemManager.cpp
  ItemManagerApp.cpp
  ItemManagerApp.h
  ItemManagerLogger.cpp
  ItemManagerLogger.h
  ItemManagerReadQueueThread.cpp
  ItemManagerReadQueueThread.h
  pch.h
  resource.h
)

if(WIN32)
    target_sources(ItemManager PRIVATE ItemManager.rc)
endif()

# Use C++23
target_compile_features(ItemManager PUBLIC cxx_std_23)

# Enable warnings as errors
if(OPENKO_COMPILE_WARNINGS_AS_ERROR)
  set_property(TARGET ItemManager PROPERTY COMPILE_WARNING_AS_ERROR ON)
endif()

# Setup PCH
target_precompile_headers(ItemManager PRIVATE pch.h)

# Setup dependencies
target_link_libraries(ItemManager
  PRIVATE
    shared-server
)

# Setup warning levels
target_compile_options(ItemManager PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W3>
  $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>
  $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Wpedantic>
)
