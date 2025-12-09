set(DB_MODELS_DIR "${OPENKO_DEPS_DIR}/db-models")

add_library(db-models STATIC
  ${DB_MODELS_DIR}/AIServer/binder/AIServerBinder.cpp
  ${DB_MODELS_DIR}/AIServer/binder/AIServerBinder.h
  ${DB_MODELS_DIR}/AIServer/model/AIServerModel.cpp
  ${DB_MODELS_DIR}/AIServer/model/AIServerModel.h
  ${DB_MODELS_DIR}/Aujard/binder/AujardBinder.cpp
  ${DB_MODELS_DIR}/Aujard/binder/AujardBinder.h
  ${DB_MODELS_DIR}/Aujard/model/AujardModel.cpp
  ${DB_MODELS_DIR}/Aujard/model/AujardModel.h
  ${DB_MODELS_DIR}/BinderUtil/BinderUtil.cpp
  ${DB_MODELS_DIR}/BinderUtil/BinderUtil.h
  ${DB_MODELS_DIR}/detail/StoredProc/StoredProcedure.cpp
  ${DB_MODELS_DIR}/detail/StoredProc/StoredProcedure.h
  ${DB_MODELS_DIR}/Ebenezer/binder/EbenezerBinder.cpp
  ${DB_MODELS_DIR}/Ebenezer/binder/EbenezerBinder.h
  ${DB_MODELS_DIR}/Ebenezer/model/EbenezerModel.cpp
  ${DB_MODELS_DIR}/Ebenezer/model/EbenezerModel.h
  ${DB_MODELS_DIR}/Full/binder/FullBinder.cpp
  ${DB_MODELS_DIR}/Full/binder/FullBinder.h
  ${DB_MODELS_DIR}/Full/model/FullModel.cpp
  ${DB_MODELS_DIR}/Full/model/FullModel.h
  ${DB_MODELS_DIR}/ModelUtil/ModelUtil.cpp
  ${DB_MODELS_DIR}/ModelUtil/ModelUtil.h
  ${DB_MODELS_DIR}/StoredProc/StoredProc.cpp
  ${DB_MODELS_DIR}/StoredProc/StoredProc.h
  ${DB_MODELS_DIR}/VersionManager/binder/VersionManagerBinder.cpp
  ${DB_MODELS_DIR}/VersionManager/binder/VersionManagerBinder.h
  ${DB_MODELS_DIR}/VersionManager/model/VersionManagerModel.cpp
  ${DB_MODELS_DIR}/VersionManager/model/VersionManagerModel.h
)

# Use C++23
target_compile_features(db-models PUBLIC cxx_std_23)

# Require nanodbc
target_link_libraries(db-models PUBLIC nanodbc)

# Expose include path
target_include_directories(db-models
  PUBLIC
    ${DB_MODELS_DIR}/
)

# Build with UTF-8. This is needed so that builds match.
target_compile_options(db-models
  PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/utf-8>
)

# Setup warning levels
target_compile_options(db-models PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W3>
  $<$<CXX_COMPILER_ID:GNU>:-Wall>
  $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra>
)
