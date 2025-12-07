set(DB_LIBRARY_DIR "${OPENKO_DEPS_DIR}/db-library")

add_library(db-library STATIC
  ${DB_LIBRARY_DIR}/Connection.cpp
  ${DB_LIBRARY_DIR}/Connection.h
  ${DB_LIBRARY_DIR}/ConnectionManager.cpp
  ${DB_LIBRARY_DIR}/ConnectionManager.h
  ${DB_LIBRARY_DIR}/DatasourceConfig.h
  ${DB_LIBRARY_DIR}/Exceptions.h
  ${DB_LIBRARY_DIR}/fwd.h
  ${DB_LIBRARY_DIR}/Model.h
  ${DB_LIBRARY_DIR}/ModelRecordSet.h
  ${DB_LIBRARY_DIR}/pch.h
  ${DB_LIBRARY_DIR}/PoolConnection.cpp
  ${DB_LIBRARY_DIR}/PoolConnection.h
  ${DB_LIBRARY_DIR}/RecordSetLoader.h
  ${DB_LIBRARY_DIR}/RecordSetLoader_STLMap.h
  ${DB_LIBRARY_DIR}/RecordsetLoader_Vector.h
  ${DB_LIBRARY_DIR}/SqlBuilder.h
  ${DB_LIBRARY_DIR}/StoredProc.h
  ${DB_LIBRARY_DIR}/utils.cpp
  ${DB_LIBRARY_DIR}/utils.h
)

# Use C++23
target_compile_features(db-library PUBLIC cxx_std_23)

# Setup PCH
target_precompile_headers(db-library PRIVATE ${DB_LIBRARY_DIR}/pch.h)

# Setup dependencies
target_link_libraries(db-library
  PUBLIC
    db-modules
    spdlog
)

# Setup warning levels
target_compile_options(db-library PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W3>
  $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>
  $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Wpedantic>
)
