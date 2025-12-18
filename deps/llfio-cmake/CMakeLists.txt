fetchcontent_declare(
  llfio
  GIT_REPOSITORY        "https://github.com/Open-KO/llfio.git"
  GIT_TAG               "20250527-OpenKO"
  GIT_PROGRESS          ON
  GIT_SHALLOW           ON
  OVERRIDE_FIND_PACKAGE TRUE
  EXCLUDE_FROM_ALL
)

message(STATUS "OpenKO: [llfio] Checking and fetching...")

fetchcontent_makeavailable(llfio)

message(STATUS "OpenKO: [llfio] Up-to-date!")

target_compile_definitions(llfio_sl INTERFACE "LLFIO_HEADERS_ONLY=0")

if(WIN32)
  target_compile_definitions(llfio_sl PUBLIC "NTKERNEL_ERROR_CATEGORY_INLINE=0")
endif()

if(MSVC)
  target_compile_definitions(llfio_sl PUBLIC "_SILENCE_CXX20_CODECVT_CHAR8_T_FACETS_DEPRECATION_WARNING")
endif()
