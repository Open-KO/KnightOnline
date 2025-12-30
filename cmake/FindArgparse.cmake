# Get Argparse package
#
# Makes the argparse target available.

include(FetchContent)

fetchcontent_declare(
  argparse
  GIT_REPOSITORY        "https://github.com/p-ranav/argparse.git"
  GIT_TAG               "v3.2"
  GIT_PROGRESS          ON
  GIT_SHALLOW           ON
  OVERRIDE_FIND_PACKAGE TRUE
  EXCLUDE_FROM_ALL
)

fetchcontent_makeavailable(argparse)

set(argparse_FOUND TRUE)
