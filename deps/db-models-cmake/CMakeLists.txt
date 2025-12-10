fetchcontent_declare(
  db_models
  GIT_REPOSITORY        "https://github.com/Open-KO/OpenKO-db-models.git"
  GIT_TAG               "v0.0.1"
  GIT_PROGRESS          ON
  GIT_SHALLOW           ON
  OVERRIDE_FIND_PACKAGE TRUE
  SOURCE_DIR            "${FETCHCONTENT_BASE_DIR}/db-models"

  EXCLUDE_FROM_ALL
)

message(STATUS "OpenKO: [db-models] Checking and fetching...")

fetchcontent_makeavailable(db_models)

message(STATUS "OpenKO: [db-models] Up-to-date!")

add_library(db-models STATIC
  "${db_models_SOURCE_DIR}/AIServer/binder/AIServerBinder.cpp"
  "${db_models_SOURCE_DIR}/AIServer/binder/AIServerBinder.h"
  "${db_models_SOURCE_DIR}/AIServer/model/AIServerModel.cpp"
  "${db_models_SOURCE_DIR}/AIServer/model/AIServerModel.h"
  "${db_models_SOURCE_DIR}/Aujard/binder/AujardBinder.cpp"
  "${db_models_SOURCE_DIR}/Aujard/binder/AujardBinder.h"
  "${db_models_SOURCE_DIR}/Aujard/model/AujardModel.cpp"
  "${db_models_SOURCE_DIR}/Aujard/model/AujardModel.h"
  "${db_models_SOURCE_DIR}/BinderUtil/BinderUtil.cpp"
  "${db_models_SOURCE_DIR}/BinderUtil/BinderUtil.h"
  "${db_models_SOURCE_DIR}/detail/StoredProc/StoredProcedure.cpp"
  "${db_models_SOURCE_DIR}/detail/StoredProc/StoredProcedure.h"
  "${db_models_SOURCE_DIR}/Ebenezer/binder/EbenezerBinder.cpp"
  "${db_models_SOURCE_DIR}/Ebenezer/binder/EbenezerBinder.h"
  "${db_models_SOURCE_DIR}/Ebenezer/model/EbenezerModel.cpp"
  "${db_models_SOURCE_DIR}/Ebenezer/model/EbenezerModel.h"
  "${db_models_SOURCE_DIR}/Full/binder/FullBinder.cpp"
  "${db_models_SOURCE_DIR}/Full/binder/FullBinder.h"
  "${db_models_SOURCE_DIR}/Full/model/FullModel.cpp"
  "${db_models_SOURCE_DIR}/Full/model/FullModel.h"
  "${db_models_SOURCE_DIR}/ModelUtil/ModelUtil.cpp"
  "${db_models_SOURCE_DIR}/ModelUtil/ModelUtil.h"
  "${db_models_SOURCE_DIR}/StoredProc/StoredProc.cpp"
  "${db_models_SOURCE_DIR}/StoredProc/StoredProc.h"
  "${db_models_SOURCE_DIR}/VersionManager/binder/VersionManagerBinder.cpp"
  "${db_models_SOURCE_DIR}/VersionManager/binder/VersionManagerBinder.h"
  "${db_models_SOURCE_DIR}/VersionManager/model/VersionManagerModel.cpp"
  "${db_models_SOURCE_DIR}/VersionManager/model/VersionManagerModel.h"
)

# Require nanodbc
target_link_libraries(db-models PUBLIC nanodbc)

# Expose include path
target_include_directories(db-models PUBLIC
  "${db_models_SOURCE_DIR}/"
)

# Build with UTF-8. This is needed so that builds match.
target_compile_options(db-models PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/utf-8>
)

# Setup warning levels
target_compile_options(db-models PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W3>
  $<$<CXX_COMPILER_ID:GNU>:-Wall>
  $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra>
)
