# Get BoostSubset package
#
# Makes the Boost targets available.

set(BOOST_ENABLE_CMAKE ON)
set(BOOST_SUBMODULES # This isn't exactly ideal, but since we only use a tiny subset of Boost it's probably manageable
  "tools/cmake;"
  "libs/assert;libs/config;libs/container;libs/interprocess;"
  "libs/intrusive;libs/move;libs/predef;libs/type_traits;"
  "libs/winapi"
)

fetchcontent_declare(
  Boost
  GIT_REPOSITORY        "https://github.com/boostorg/boost.git"
  GIT_TAG               "boost-1.90.0"
  GIT_SUBMODULES        "${BOOST_SUBMODULES}"
  GIT_PROGRESS          TRUE
  GIT_SHALLOW           TRUE
  OVERRIDE_FIND_PACKAGE TRUE
  EXCLUDE_FROM_ALL
)

fetchcontent_makeavailable(Boost)
find_package(Boost 1.89.0 EXACT REQUIRED COMPONENTS interprocess)

set(boostsubset_FOUND TRUE)
