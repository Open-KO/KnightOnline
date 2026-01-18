# Get Spdlog package
#
# Makes the spdlog target available.

fetchcontent_declare(
  spdlog
  GIT_REPOSITORY        "https://github.com/Open-KO/spdlog.git"
  GIT_TAG               "v1.15.3b-OpenKO"
  GIT_PROGRESS          ON
  GIT_SHALLOW           ON
  EXCLUDE_FROM_ALL
)

set(SPDLOG_ENABLE_PCH ON CACHE BOOL "spdlog: Enable PCH")

fetchcontent_makeavailable(spdlog)

set(spdlog_FOUND TRUE)
