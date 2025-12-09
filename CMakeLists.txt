cmake_minimum_required(VERSION 3.28.3)
include(FetchContent)

project(OpenKO
  VERSION 0.0.1
)

# Config
option(OPENKO_BUILD_SERVERS "Build the server projects (default on)" ON)
option(OPENKO_COMPILE_WARNINGS_AS_ERROR "Enable treating warnings as errors (default on)" ON)

if(WIN32)
  # Set the minimum Windows version
  set(_WIN32_WINNT "0x0A00" CACHE STRING "Minimum Windows version (default 0x0A00 - Windows 10)")
  add_definitions(-D_WIN32_WINNT=${_WIN32_WINNT})
endif()

set(SPDLOG_ENABLE_PCH ON CACHE BOOL "Enable PCH in spdlog")
set(NANODBC_FORCE_LIBCXX OFF CACHE BOOL "Force the use of libc++ (default off)")
set(NANODBC_DISABLE_MSSQL_TVP ON "Disable MSSQL Table-valued parameters in nanodbc")
set(NANODBC_DISABLE_NULL_ACCESS_ERROR ON "Disable null_access_error() exceptions in nanodbc")
set(NANODBC_USE_UINT8_FOR_TINYINT ON "Enable use of uint8 to represent TINYINT fields instead of int16 in nanodbc")

# Setup dependency dir
set(OPENKO_DEPS_DIR ${CMAKE_SOURCE_DIR}/deps)

# Add dependencies
add_subdirectory(deps/asio-cmake)
add_subdirectory(deps/spdlog)
add_subdirectory(shared)

include_directories(
  ${CMAKE_SOURCE_DIR} # repo root, for <shared/...>
  ${OPENKO_DEPS_DIR}  # dependencies like asio, spdlog, etc.
)

if(OPENKO_BUILD_SERVERS)
  ###############
  # Dependencies
  ###############
  set(BOOST_ENABLE_CMAKE ON)
  set(BOOST_SUBMODULES # This isn't exactly ideal, but since we only use a tiny subset of Boost it's probably manageable
      "tools/cmake;"
      "libs/assert;libs/config;libs/container;libs/interprocess;"
      "libs/intrusive;libs/move;libs/predef;libs/type_traits;"
      "libs/winapi"
  )

  fetchcontent_declare(
      Boost
      GIT_REPOSITORY "https://github.com/boostorg/boost.git"
      GIT_TAG "boost-1.89.0"
      GIT_SUBMODULES ${BOOST_SUBMODULES}
      GIT_PROGRESS ON
      GIT_SHALLOW ON
      OVERRIDE_FIND_PACKAGE TRUE
      EXCLUDE_FROM_ALL
  )

  fetchcontent_makeavailable(Boost)
  find_package(Boost 1.89.0 EXACT REQUIRED COMPONENTS interprocess)

  add_subdirectory(deps/FTXUI)

  add_subdirectory(deps/nanodbc)
  target_compile_features(nanodbc PUBLIC cxx_std_20)

  # codecvt conversions are being deprecated.
  # For now we'll just ignore the warning and hope they fix it before it's removed.
  target_compile_options(nanodbc
    PRIVATE
      $<$<CXX_COMPILER_ID:Clang>:-Wno-deprecated-declarations>
  )

  add_subdirectory(deps/db-modules-cmake)
  add_subdirectory(deps/db-library-cmake)

  add_subdirectory(Server/shared-server)
  
  ###############
  # Servers
  ###############
  add_subdirectory(Server/AIServer)
  add_subdirectory(Server/Aujard)
  add_subdirectory(Server/Ebenezer)
  add_subdirectory(Server/ItemManager)
  add_subdirectory(Server/VersionManager)
endif()
