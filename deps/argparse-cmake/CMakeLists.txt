fetchcontent_declare(
  argparse
  GIT_REPOSITORY        "https://github.com/p-ranav/argparse.git"
  GIT_TAG               "v3.2"
  GIT_PROGRESS          ON
  GIT_SHALLOW           ON
  OVERRIDE_FIND_PACKAGE TRUE
  EXCLUDE_FROM_ALL
)

message(STATUS "OpenKO: [argparse] Checking and fetching...")

fetchcontent_makeavailable(argparse)

message(STATUS "OpenKO: [argparse] Up-to-date!")
