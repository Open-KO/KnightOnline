fetchcontent_declare(
  googletest
  GIT_REPOSITORY        "https://github.com/google/googletest.git"
  GIT_TAG               "v1.17.0"
  GIT_PROGRESS          ON
  GIT_SHALLOW           ON
  OVERRIDE_FIND_PACKAGE TRUE
  EXCLUDE_FROM_ALL
)

message(STATUS "OpenKO: [googletest] Checking and fetching...")

fetchcontent_makeavailable(googletest)

message(STATUS "OpenKO: [googletest] Up-to-date!")
