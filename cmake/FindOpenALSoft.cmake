# Get OpenALSoft package
#
# Makes the openal-soft target available.

fetchcontent_declare(
  openalsoft
  GIT_REPOSITORY        "https://github.com/Open-KO/openal-soft.git"
  GIT_TAG               "1.25.0-32f75f"
  GIT_PROGRESS          ON
  GIT_SHALLOW           ON
  EXCLUDE_FROM_ALL
)

set(ALSOFT_UTILS OFF CACHE BOOL "openal-soft: Build utility programs")
set(ALSOFT_EXAMPLES OFF CACHE BOOL "openal-soft: Build example programs")
set(ALSOFT_UTILS OFF CACHE BOOL "openal-soft: Build utility programs")
set(ALSOFT_ENABLE_MODULES OFF CACHE BOOL "openal-soft: Enable use of C++20 modules when supported")

if(MSVC)
  set(FORCE_STATIC_VCRT ON CACHE BOOL "openal-soft: Force /MT for static VC runtimes")
  set(LIBTYPE STATIC CACHE STRING "openal-soft: Library type")
endif()

fetchcontent_makeavailable(openalsoft)

set(openalsoft_FOUND TRUE)
