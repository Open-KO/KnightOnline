# Get client-assets package

set(OPENKO_CLIENT_DIR "${CMAKE_BINARY_DIR}/ClientData" CACHE STRING "Client path")

# Ensure the client directory exists.
file(MAKE_DIRECTORY "${OPENKO_CLIENT_DIR}")


if(OPENKO_FETCH_CLIENT_ASSETS)
  fetchcontent_declare(
    client_assets
    GIT_REPOSITORY        "https://github.com/Open-KO/ko-client-assets.git"
    GIT_TAG               "v1.298.0"
    GIT_PROGRESS          ON
    GIT_SHALLOW           ON
    SOURCE_DIR            "${OPENKO_CLIENT_DIR}"

    EXCLUDE_FROM_ALL
  )

  fetchcontent_makeavailable(client_assets)
else()
  message(STATUS "OpenKO: Skipped fetching assets due to OPENKO_FETCH_CLIENT_ASSETS=${OPENKO_FETCH_CLIENT_ASSETS}")
endif()

# As it's a user config file, Server.ini is not directly available in the repo.
# Instead, we have a default file we should copy (Server.ini.default).
# This ensures we can use safe defaults without risking contamination in the repo.
set(DEFAULT_CLIENT_SERVER_INI "${OPENKO_CLIENT_DIR}/Server.ini.default")
set(TARGET_CLIENT_SERVER_INI "${OPENKO_CLIENT_DIR}/Server.ini")

if(NOT EXISTS "${TARGET_CLIENT_SERVER_INI}" AND EXISTS "${DEFAULT_CLIENT_SERVER_INI}")
  configure_file("${DEFAULT_CLIENT_SERVER_INI}" "${TARGET_CLIENT_SERVER_INI}" COPYONLY)
endif()
