# Get djb2 package
#
# Makes the djb2 target available.

fetchcontent_declare(
  djb2
  GIT_REPOSITORY        "https://github.com/Open-KO/djb2.git"
  GIT_TAG               "v0.0.2-OpenKO"
  GIT_PROGRESS          ON
  GIT_SHALLOW           ON
  SOURCE_SUBDIR         "MISSING_DIRECTORY" # intentionally nonexistent to bypass their CMakeLists.txt
  SOURCE_DIR            "${FETCHCONTENT_BASE_DIR}/djb2"

  EXCLUDE_FROM_ALL
)

fetchcontent_makeavailable(djb2)

add_library(djb2 INTERFACE
  "${djb2_SOURCE_DIR}/djb2_hasher.h"
)

# Expose include path
target_include_directories(djb2 INTERFACE
  "${djb2_SOURCE_DIR}/../" # for <djb2/...>
)
