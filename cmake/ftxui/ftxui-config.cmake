# Get ftxui package
#
# Makes the ftxui::component, ftxui::dom, ftxui::screen targets available.

fetchcontent_declare(
  ftxui
  GIT_REPOSITORY        "https://github.com/ArthurSonzogni/FTXUI.git"
  GIT_TAG               "v6.1.9"
  GIT_PROGRESS          ON
  GIT_SHALLOW           ON
  OVERRIDE_FIND_PACKAGE TRUE
  EXCLUDE_FROM_ALL
)

fetchcontent_makeavailable(ftxui)
