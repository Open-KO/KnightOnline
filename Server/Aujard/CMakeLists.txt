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

# Use C++23
target_compile_features(Aujard PUBLIC cxx_std_23)

# Enable warnings as errors
if(OPENKO_COMPILE_WARNINGS_AS_ERROR)
  set_property(TARGET Aujard PROPERTY COMPILE_WARNING_AS_ERROR ON)
endif()

# Setup PCH
target_precompile_headers(Aujard PRIVATE pch.h)

# Setup dependencies
target_link_libraries(Aujard
  PRIVATE
    shared-server
    db-library
)

# Setup warning levels
target_compile_options(Aujard PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W3>
  $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic -Wno-gnu-anonymous-struct -Wno-nested-anon-types>
  $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Wpedantic -Wno-gnu-anonymous-struct -Wno-nested-anon-types>
)
