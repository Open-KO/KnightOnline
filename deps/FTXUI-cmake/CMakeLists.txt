fetchcontent_declare(
  ftxui
  GIT_REPOSITORY        "https://github.com/ArthurSonzogni/FTXUI.git"
  GIT_TAG               "v6.1.9"
  GIT_PROGRESS          ON
  GIT_SHALLOW           ON
  OVERRIDE_FIND_PACKAGE TRUE
  EXCLUDE_FROM_ALL
)
message(STATUS "OpenKO: [ftxui] Checking and fetching...")

fetchcontent_makeavailable(ftxui)

message(STATUS "OpenKO: [ftxui] Up-to-date!")
