add_library(db-library STATIC
  Connection.cpp
  Connection.h
  ConnectionManager.cpp
  ConnectionManager.h
  DatasourceConfig.h
  Exceptions.h
  fwd.h
  Model.h
  ModelRecordSet.h
  pch.h
  PoolConnection.cpp
  PoolConnection.h
  RecordSetLoader.h
  RecordSetLoader_STLMap.h
  RecordSetLoader_Vector.h
  SqlBuilder.h
  StoredProc.h
  utils.cpp
  utils.h
)

# Setup PCH
target_precompile_headers(db-library PRIVATE pch.h)

# Setup dependencies
target_link_libraries(db-library
  PUBLIC
    db-models
    spdlog
)

# Expose include path
target_include_directories(db-library INTERFACE
  "${CMAKE_CURRENT_SOURCE_DIR}/../"
)

# Setup warning levels
target_compile_options(db-library PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W3>
  $<$<CXX_COMPILER_ID:GNU>:-Wall>
  $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra>
)
