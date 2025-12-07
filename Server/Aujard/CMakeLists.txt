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

# Setup PCH
target_precompile_headers(Aujard PRIVATE pch.h)

# Setup dependencies
target_link_libraries(Aujard
  PRIVATE
    shared-server
    db-library
)
