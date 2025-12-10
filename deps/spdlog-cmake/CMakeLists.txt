fetchcontent_declare(
  spdlog
  GIT_REPOSITORY        "https://github.com/Open-KO/spdlog.git"
  GIT_TAG               "v1.15.3b-OpenKO"
  GIT_PROGRESS          ON
  GIT_SHALLOW           ON
  OVERRIDE_FIND_PACKAGE TRUE
  EXCLUDE_FROM_ALL
)

set(SPDLOG_ENABLE_PCH ON CACHE BOOL "Enable PCH in spdlog")

message(STATUS "OpenKO: [spdlog] Checking and fetching...")

fetchcontent_makeavailable(spdlog)

message(STATUS "OpenKO: [spdlog] Up-to-date!")
