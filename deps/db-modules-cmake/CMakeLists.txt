set(CMAKE_CXX_EXTENSIONS FALSE)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN TRUE)

set(DB_MODULES_DIR "${OPENKO_DEPS_DIR}/db-modules")

add_library(db-modules STATIC)

target_sources(db-modules
  PUBLIC
    FILE_SET CXX_MODULES
    BASE_DIRS ${OPENKO_DEPS_DIR}/db-modules
      FILES
        ${DB_MODULES_DIR}/AIServer/binder/AIServerBinder.ixx
        ${DB_MODULES_DIR}/AIServer/model/AIServerModel.ixx
        ${DB_MODULES_DIR}/Aujard/binder/AujardBinder.ixx
        ${DB_MODULES_DIR}/Aujard/model/AujardModel.ixx
        ${DB_MODULES_DIR}/BinderUtil/BinderUtil.ixx
        ${DB_MODULES_DIR}/Ebenezer/binder/EbenezerBinder.ixx
        ${DB_MODULES_DIR}/Ebenezer/model/EbenezerModel.ixx
        ${DB_MODULES_DIR}/Full/binder/FullBinder.ixx
        ${DB_MODULES_DIR}/Full/model/FullModel.ixx
        ${DB_MODULES_DIR}/ModelUtil/ModelUtil.ixx
        ${DB_MODULES_DIR}/StoredProc/StoredProc.ixx
        ${DB_MODULES_DIR}/VersionManager/binder/VersionManagerBinder.ixx
        ${DB_MODULES_DIR}/VersionManager/model/VersionManagerModel.ixx
)

# Use C++23
target_compile_features(db-modules PUBLIC cxx_std_23)

# Require nanodbc
target_link_libraries(db-modules PUBLIC nanodbc)

# Build with UTF-8. This is needed so that builds match.
target_compile_options(db-modules PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/utf-8>
)
