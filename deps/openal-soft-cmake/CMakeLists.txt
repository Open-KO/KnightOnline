fetchcontent_declare(
  openalsoft
  GIT_REPOSITORY        "https://github.com/Open-KO/openal-soft.git"
  GIT_TAG               "1.25.0-1cdd3c"
  GIT_PROGRESS          ON
  GIT_SHALLOW           ON
  OVERRIDE_FIND_PACKAGE TRUE
  EXCLUDE_FROM_ALL
)

set(ALSOFT_UTILS OFF CACHE BOOL "openal-soft: Build utility programs")
set(ALSOFT_EXAMPLES OFF CACHE BOOL "openal-soft: Build example programs")
set(ALSOFT_UTILS OFF CACHE BOOL "openal-soft: Build utility programs")
set(ALSOFT_ENABLE_MODULES OFF CACHE BOOL "openal-soft: Enable use of C++20 modules when supported")

if(MSVC)
  set(FORCE_STATIC_VCRT ON CACHE BOOL "openal-soft: Force /MT for static VC runtimes")
endif()

message(STATUS "OpenKO: [openal-soft] Checking and fetching...")

fetchcontent_makeavailable(openalsoft)

message(STATUS "OpenKO: [openal-soft] Up-to-date!")
