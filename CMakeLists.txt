cmake_minimum_required(VERSION 3.28.3)

project(OpenKO
  VERSION 0.0.1
)

find_package(Git REQUIRED)
include(FetchContent)

# Set minimum standard to C++23 (unless otherwise specified)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# We don't use modules, so there's no reason to scan
set(CXX_SCAN_FOR_MODULES OFF)

# Config
option(OPENKO_BUILD_SERVERS "Build the server projects (default on)" ON)
option(OPENKO_COMPILE_WARNINGS_AS_ERROR "Enable treating warnings as errors (default on)" ON)

# Define the bin directory; defaults to repo/bin
set(OPENKO_BIN_DIR "${CMAKE_BINARY_DIR}/bin" CACHE PATH "Directory for final binaries")

if(WIN32)
  # Set the minimum Windows version
  set(_WIN32_WINNT "0x0A00" CACHE STRING "Minimum Windows version (default 0x0A00 - Windows 10)")
  add_definitions(-D_WIN32_WINNT=${_WIN32_WINNT})
endif()

message(STATUS "OpenKO feature: OPENKO_BUILD_SERVERS - ${OPENKO_BUILD_SERVERS}")
message(STATUS "OpenKO feature: OPENKO_COMPILE_WARNINGS_AS_ERROR - ${OPENKO_COMPILE_WARNINGS_AS_ERROR}")
message(STATUS "OpenKO feature: OPENKO_BIN_DIR - ${OPENKO_BIN_DIR}")

if(WIN32)
  message(STATUS "OpenKO feature: _WIN32_WINNT - ${_WIN32_WINNT}")
endif()

get_property(is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

# Multiconfig generators don't require a build type at config time.
# They generate multiple configurations.
# The output directory will have the build type auto-appended.
if(is_multi_config)
  # Set output directory
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${OPENKO_BIN_DIR}")
else()
  # Default build type to Debug
  if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "OpenKO: Defaulting build type to 'Debug' as none was specified via -DCMAKE_BUILD_TYPE (expected build types: Debug, Release, RelWithDebugInfo, MinSizeRel)")

    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
  endif()

  # Set output directory
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${OPENKO_BIN_DIR}/${CMAKE_BUILD_TYPE}")
endif()

set(OPENKO_SERVER_MAP_SRC_DIR "${CMAKE_SOURCE_DIR}/Server/bin/MAP")
set(OPENKO_SERVER_MAP_DST_DIR "${OPENKO_BIN_DIR}/MAP")
set(OPENKO_SERVER_QUESTS_SRC_DIR "${CMAKE_SOURCE_DIR}/Server/bin/QUESTS")
set(OPENKO_SERVER_QUESTS_DST_DIR "${OPENKO_BIN_DIR}/QUESTS")

# Set the default solution folder to place targets/projects in
set(CMAKE_FOLDER "_deps")

# Add dependencies
add_subdirectory(deps/asio-cmake)
add_subdirectory(deps/spdlog-cmake)

set(CMAKE_FOLDER) # reset so shared is placed in the root
add_subdirectory(shared)

include_directories(
  "${CMAKE_SOURCE_DIR}" # repo root, for <shared/...>
)

if(OPENKO_BUILD_SERVERS)
  message(STATUS "OpenKO: Preparing to configure servers...")

  # Dependencies
  set(CMAKE_FOLDER "Server/_deps/boost")
  add_subdirectory(deps/boost-cmake EXCLUDE_FROM_ALL)

  set(CMAKE_FOLDER "Server/_deps/ftxui")
  add_subdirectory(deps/FTXUI-cmake EXCLUDE_FROM_ALL)

  set(CMAKE_FOLDER "Server/_deps")
  add_subdirectory(deps/nanodbc-cmake EXCLUDE_FROM_ALL)

  add_subdirectory(deps/db-models-cmake EXCLUDE_FROM_ALL)
  add_subdirectory(deps/db-library-cmake EXCLUDE_FROM_ALL)
  add_subdirectory(deps/djb2-cmake EXCLUDE_FROM_ALL)
  add_subdirectory(Server/shared-server)

  add_custom_target(copy-server-assets ALL
    COMMENT "Copying server assets"
    COMMAND "${CMAKE_COMMAND}" -E echo "Copying server MAP dir to ${OPENKO_SERVER_MAP_DST_DIR}"
    COMMAND "${CMAKE_COMMAND}" -E copy_directory "${OPENKO_SERVER_MAP_SRC_DIR}" "${OPENKO_SERVER_MAP_DST_DIR}"
    COMMAND "${CMAKE_COMMAND}" -E echo "Copying server QUESTS dir to ${OPENKO_SERVER_QUESTS_DST_DIR}"
    COMMAND "${CMAKE_COMMAND}" -E copy_directory "${OPENKO_SERVER_QUESTS_SRC_DIR}" "${OPENKO_SERVER_QUESTS_DST_DIR}"
    COMMAND "${CMAKE_COMMAND}" -E echo "Server assets copied."
  )

  # Servers
  set(CMAKE_FOLDER "Server")
  add_subdirectory(Server/AIServer)
  add_subdirectory(Server/Aujard)
  add_subdirectory(Server/Ebenezer)
  add_subdirectory(Server/ItemManager)
  add_subdirectory(Server/VersionManager)
endif()
