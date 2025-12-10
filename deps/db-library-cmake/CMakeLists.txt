fetchcontent_declare(
  db_library
  GIT_REPOSITORY        "https://github.com/Open-KO/OpenKO-db-library.git"
  GIT_TAG               "v0.0.1"
  GIT_PROGRESS          ON
  GIT_SHALLOW           ON
  OVERRIDE_FIND_PACKAGE TRUE
  SOURCE_DIR            "${FETCHCONTENT_BASE_DIR}/db-library"

  EXCLUDE_FROM_ALL
)

message(STATUS "OpenKO: [db-library] Checking and fetching...")

fetchcontent_makeavailable(db_library)

message(STATUS "OpenKO: [db-library] Up-to-date!")

add_library(db-library STATIC
  "${db_library_SOURCE_DIR}/Connection.cpp"
  "${db_library_SOURCE_DIR}/Connection.h"
  "${db_library_SOURCE_DIR}/ConnectionManager.cpp"
  "${db_library_SOURCE_DIR}/ConnectionManager.h"
  "${db_library_SOURCE_DIR}/DatasourceConfig.h"
  "${db_library_SOURCE_DIR}/Exceptions.h"
  "${db_library_SOURCE_DIR}/fwd.h"
  "${db_library_SOURCE_DIR}/Model.h"
  "${db_library_SOURCE_DIR}/ModelRecordSet.h"
  "${db_library_SOURCE_DIR}/pch.h"
  "${db_library_SOURCE_DIR}/PoolConnection.cpp"
  "${db_library_SOURCE_DIR}/PoolConnection.h"
  "${db_library_SOURCE_DIR}/RecordSetLoader.h"
  "${db_library_SOURCE_DIR}/RecordSetLoader_STLMap.h"
  "${db_library_SOURCE_DIR}/RecordSetLoader_Vector.h"
  "${db_library_SOURCE_DIR}/SqlBuilder.h"
  "${db_library_SOURCE_DIR}/StoredProc.h"
  "${db_library_SOURCE_DIR}/utils.cpp"
  "${db_library_SOURCE_DIR}/utils.h"
)

# Setup PCH
target_precompile_headers(db-library PRIVATE "${db_library_SOURCE_DIR}/pch.h")

# Setup dependencies
target_link_libraries(db-library
  PUBLIC
    db-models
    spdlog
)

# Expose include path
target_include_directories(db-library INTERFACE
  "${db_library_SOURCE_DIR}/../"
)

# Setup warning levels
target_compile_options(db-library PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W3>
  $<$<CXX_COMPILER_ID:GNU>:-Wall>
  $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra>
)
