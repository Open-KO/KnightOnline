set(DB_MODULES_DIR "${OPENKO_DEPS_DIR}/db-modules")

add_library(db-modules STATIC
  ${DB_MODULES_DIR}/AIServer/binder/AIServerBinder.cpp
  ${DB_MODULES_DIR}/AIServer/binder/AIServerBinder.h
  ${DB_MODULES_DIR}/AIServer/model/AIServerModel.cpp
  ${DB_MODULES_DIR}/AIServer/model/AIServerModel.h
  ${DB_MODULES_DIR}/Aujard/binder/AujardBinder.cpp
  ${DB_MODULES_DIR}/Aujard/binder/AujardBinder.h
  ${DB_MODULES_DIR}/Aujard/model/AujardModel.cpp
  ${DB_MODULES_DIR}/Aujard/model/AujardModel.h
  ${DB_MODULES_DIR}/BinderUtil/BinderUtil.cpp
  ${DB_MODULES_DIR}/BinderUtil/BinderUtil.h
  ${DB_MODULES_DIR}/Ebenezer/binder/EbenezerBinder.cpp
  ${DB_MODULES_DIR}/Ebenezer/binder/EbenezerBinder.h
  ${DB_MODULES_DIR}/Ebenezer/model/EbenezerModel.cpp
  ${DB_MODULES_DIR}/Ebenezer/model/EbenezerModel.h
  ${DB_MODULES_DIR}/Full/binder/FullBinder.cpp
  ${DB_MODULES_DIR}/Full/binder/FullBinder.h
  ${DB_MODULES_DIR}/Full/model/FullModel.cpp
  ${DB_MODULES_DIR}/Full/model/FullModel.h
  ${DB_MODULES_DIR}/ModelUtil/ModelUtil.cpp
  ${DB_MODULES_DIR}/ModelUtil/ModelUtil.h
  ${DB_MODULES_DIR}/StoredProc/StoredProc.cpp
  ${DB_MODULES_DIR}/StoredProc/StoredProc.h
  ${DB_MODULES_DIR}/StoredProc/detail/StoredProcedure.cpp
  ${DB_MODULES_DIR}/StoredProc/detail/StoredProcedure.h
  ${DB_MODULES_DIR}/VersionManager/binder/VersionManagerBinder.cpp
  ${DB_MODULES_DIR}/VersionManager/binder/VersionManagerBinder.h
  ${DB_MODULES_DIR}/VersionManager/model/VersionManagerModel.cpp
  ${DB_MODULES_DIR}/VersionManager/model/VersionManagerModel.h
)

# Use C++23
target_compile_features(db-modules PUBLIC cxx_std_23)

# Require nanodbc
target_link_libraries(db-modules PUBLIC nanodbc)

# Expose include path
target_include_directories(db-modules
  PUBLIC
    ${DB_MODULES_DIR}/
)

# Build with UTF-8. This is needed so that builds match.
target_compile_options(db-modules
  PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/utf-8>
)
