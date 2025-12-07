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

# Setup PCH
target_precompile_headers(ItemManager PRIVATE pch.h)

# Setup dependencies
target_link_libraries(ItemManager
  PRIVATE
    shared-server
)
