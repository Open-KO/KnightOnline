# Get Googletest package
#
# Makes the gtest, gtest_main, gmock, gmock_main targets available.

fetchcontent_declare(
  googletest
  GIT_REPOSITORY        "https://github.com/google/googletest.git"
  GIT_TAG               "v1.17.0"
  GIT_PROGRESS          ON
  GIT_SHALLOW           ON
  OVERRIDE_FIND_PACKAGE TRUE
  EXCLUDE_FROM_ALL
)

fetchcontent_makeavailable(googletest)

set(googletest_FOUND TRUE)
