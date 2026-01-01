cmake_minimum_required(VERSION 3.28.3)

project(OpenKO
  VERSION 0.0.1
)

find_package(Git REQUIRED)

# Search in the "cmake" directory for additional CMake modules.
set(OPENKO_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
list(APPEND CMAKE_MODULE_PATH "${OPENKO_MODULE_PATH}")

include(FetchContent)
include(RequirePackage)

# Set minimum standard to C++23 (unless otherwise specified)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# We don't use modules, so there's no reason to scan
set(CXX_SCAN_FOR_MODULES OFF)

if(MSVC)
  # Statically link to the MSVC runtime
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  
  # Ensure consistent charset builds across projects
  add_compile_options(/utf-8)
endif()

# Config
option(OPENKO_BUILD_CLIENT "Build the client projects - MSVC only (default on)" ON)
option(OPENKO_BUILD_CLIENT_TOOLS "Build the client tool projects - MSVC only (default on)" ON)
option(OPENKO_BUILD_TOOLS "Build the development tool projects - MSVC only (default on)" ON)
option(OPENKO_BUILD_SERVERS "Build the server projects (default on)" ON)
option(OPENKO_BUILD_TESTS "Build the test projects (default on)" ON)
option(OPENKO_COMPILE_WARNINGS_AS_ERROR "Enable treating warnings as errors (default on)" ON)
option(OPENKO_FETCH_CLIENT_ASSETS "Enable fetching client assets (default on)" ON)

# Define the bin directory; defaults to repo/bin
set(OPENKO_BIN_DIR "${CMAKE_BINARY_DIR}/bin" CACHE PATH "Directory for final binaries")

if(WIN32)
  # Set the minimum Windows version
  set(_WIN32_WINNT "0x0A00" CACHE STRING "Minimum Windows version (default 0x0A00 - Windows 10)")
  add_definitions(-D_WIN32_WINNT=${_WIN32_WINNT})
endif()

message(STATUS "OpenKO feature: OPENKO_BUILD_CLIENT - ${OPENKO_BUILD_CLIENT}")
message(STATUS "OpenKO feature: OPENKO_BUILD_CLIENT_TOOLS - ${OPENKO_BUILD_CLIENT_TOOLS}")
message(STATUS "OpenKO feature: OPENKO_BUILD_TOOLS - ${OPENKO_BUILD_TOOLS}")
message(STATUS "OpenKO feature: OPENKO_BUILD_SERVERS - ${OPENKO_BUILD_SERVERS}")
message(STATUS "OpenKO feature: OPENKO_BUILD_TESTS - ${OPENKO_BUILD_TESTS}")
message(STATUS "OpenKO feature: OPENKO_COMPILE_WARNINGS_AS_ERROR - ${OPENKO_COMPILE_WARNINGS_AS_ERROR}")
message(STATUS "OpenKO feature: OPENKO_FETCH_CLIENT_ASSETS - ${OPENKO_FETCH_CLIENT_ASSETS}")
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

set(OPENKO_CLIENT_DIR "${CMAKE_BINARY_DIR}/ClientData" CACHE STRING "Client path")
set(OPENKO_SERVER_MAP_SRC_DIR "${CMAKE_SOURCE_DIR}/assets/Server/MAP")
set(OPENKO_SERVER_MAP_DST_DIR "${OPENKO_BIN_DIR}/MAP")
set(OPENKO_SERVER_QUESTS_SRC_DIR "${CMAKE_SOURCE_DIR}/assets/Server/QUESTS")
set(OPENKO_SERVER_QUESTS_DST_DIR "${OPENKO_BIN_DIR}/QUESTS")

# Ensure the client directory exists.
file(MAKE_DIRECTORY "${OPENKO_CLIENT_DIR}")

# Generate .gitignore in build directory
if(NOT PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  file(WRITE "${PROJECT_BINARY_DIR}/.gitignore" "*")
endif()

# Add dependencies
set(CMAKE_FOLDER "_deps")
require_package(Asio)
require_package(LLFIO)
require_package(Spdlog)

include_directories(
  "${CMAKE_SOURCE_DIR}" # repo root, for <shared/...>
)

set(CMAKE_FOLDER) # reset so shared is placed in the root
add_subdirectory(src/shared)
add_subdirectory(src/FileIO)
add_subdirectory(src/MathUtils)

# Client & tool projects are currently only supported with Windows+MSVC builds.
if(MSVC)
  if(OPENKO_BUILD_CLIENT OR OPENKO_BUILD_TOOLS)
    # Dependencies
    set(CMAKE_FOLDER "N3Base/_deps")
    require_config_package(dx9sdk)

    set(CMAKE_FOLDER "N3Base/_deps/mpg123")
	require_package(Mpg123)

    set(CMAKE_FOLDER "N3Base/_deps/openal-soft")
    require_package(OpenALSoft)

    set(CMAKE_FOLDER "N3Base")
    add_subdirectory(src/N3Base)
  endif()

  if(OPENKO_BUILD_CLIENT OR OPENKO_BUILD_CLIENT_TOOLS)
    set(CMAKE_FOLDER "Client/_deps")

    # We technically require this for the Launcher & Option as well as the client itself (WarFare),
	# so we'll pull this in here.
    require_config_package(client-assets)

    # libjpeg-turbo doesn't allow regular builds. They insist on manual/external builds,
    # so we must structure this a little bit differently to everything else.
    require_package(JpegTurbo)
    add_subdirectory(src/Client/JpegFile)
  endif()

  if(OPENKO_BUILD_CLIENT)
    message(STATUS "OpenKO: Preparing to configure client...")

    set(CMAKE_FOLDER "Client")
    add_subdirectory(src/Client/WarFare)
  endif()

  if(OPENKO_BUILD_CLIENT_TOOLS)
    message(STATUS "OpenKO: Preparing to configure client tools...")

    set(CMAKE_FOLDER "Client tools/_deps")
    require_package(Zlib)
    add_subdirectory(src/Client/ZipArchive)

    set(CMAKE_FOLDER "Client tools")
    add_subdirectory(src/Client/KscViewer)
    add_subdirectory(src/Client/Launcher)
    add_subdirectory(src/Client/Option)
  endif()

  if(OPENKO_BUILD_TOOLS)
    message(STATUS "OpenKO: Preparing to configure development tools...")

    set(CMAKE_FOLDER "Tools")
    add_subdirectory(src/Tools/N3CE)
    add_subdirectory(src/Tools/N3FXE)
    add_subdirectory(src/Tools/N3ME)
    add_subdirectory(src/Tools/N3TexViewer)
    add_subdirectory(src/Tools/N3Viewer)
    add_subdirectory(src/Tools/SkyViewer)
    add_subdirectory(src/Tools/TblEditor)
    add_subdirectory(src/Tools/UIE)
  endif()
endif()

if(OPENKO_BUILD_SERVERS)
  message(STATUS "OpenKO: Preparing to configure servers...")

  # Dependencies
  set(CMAKE_FOLDER "Server/_deps/boost")
  require_package(BoostSubset)

  set(CMAKE_FOLDER "Server/_deps/ftxui")
  require_package(FTXUI)

  set(CMAKE_FOLDER "Server/_deps/argparse")
  require_package(Argparse)

  set(CMAKE_FOLDER "Server/_deps")
  require_package(Nanodbc)
  require_config_package(db-models)
  require_config_package(djb2)
  add_subdirectory(src/db-library)
  add_subdirectory(src/Server/shared-server)

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
  add_subdirectory(src/Server/AIServer)
  add_subdirectory(src/Server/Aujard)
  add_subdirectory(src/Server/Ebenezer)
  add_subdirectory(src/Server/ItemManager)
  add_subdirectory(src/Server/VersionManager)
endif()

if(OPENKO_BUILD_TESTS)
  include(CTest)

  set(CMAKE_FOLDER "Tests/_deps")
  require_package(Googletest)

  set(CMAKE_FOLDER "Tests")
  add_subdirectory(tests/FileIO)
  add_subdirectory(tests/MathUtils)
endif()
