add_executable(Ebenezer.Tests
  ItemUpgrade_test.cpp
)

target_link_libraries(Ebenezer.Tests PRIVATE
  gtest
  gtest_main
  Ebenezer.Core)

add_test(
  NAME EbenezerTests
  COMMAND Ebenezer.Tests)
