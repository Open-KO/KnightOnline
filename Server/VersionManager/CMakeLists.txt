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

# Use C++23
target_compile_features(VersionManager PUBLIC cxx_std_23)

# Setup PCH
target_precompile_headers(VersionManager PRIVATE pch.h)

# Setup dependencies
target_link_libraries(VersionManager
  PRIVATE
    shared-server
    db-library
)
