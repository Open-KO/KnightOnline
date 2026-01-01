# Get FTXUI package
#
# Makes the ftxui::component, ftxui::dom, ftxui::screen targets available.

fetchcontent_declare(
  ftxui
  GIT_REPOSITORY        "https://github.com/ArthurSonzogni/FTXUI.git"
  GIT_TAG               "v6.1.9"
  GIT_PROGRESS          ON
  GIT_SHALLOW           ON
  EXCLUDE_FROM_ALL
)

fetchcontent_makeavailable(ftxui)

set(ftxui_FOUND TRUE)
