fetchcontent_declare(
  djb2
  GIT_REPOSITORY        "https://github.com/Open-KO/djb2.git"
  GIT_TAG               "v0.0.2-OpenKO"
  GIT_PROGRESS          ON
  GIT_SHALLOW           ON
  OVERRIDE_FIND_PACKAGE TRUE
  SOURCE_SUBDIR         "MISSING_DIRECTORY" # intentionally nonexistent to bypass their CMakeLists.txt
  SOURCE_DIR            "${FETCHCONTENT_BASE_DIR}/djb2"

  EXCLUDE_FROM_ALL
)

message(STATUS "OpenKO: [djb2] Checking and fetching...")

fetchcontent_makeavailable(djb2)

message(STATUS "OpenKO: [djb2] Up-to-date!")

add_library(djb2 INTERFACE
  "${djb2_SOURCE_DIR}/djb2_hasher.h"
)

# Expose include path
target_include_directories(djb2 INTERFACE
  "${djb2_SOURCE_DIR}/../"
)
