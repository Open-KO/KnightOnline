# NOTE: Unused - no CMake client builds currently.
add_executable(client-tests
  hello_test.cpp)

target_link_libraries(client-tests PRIVATE
  gtest
  gtest_main)

add_test(
  NAME ClientTests
  COMMAND client-tests)
