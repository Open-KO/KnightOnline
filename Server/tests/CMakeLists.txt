add_executable(server-tests
  hello_test.cpp)

target_link_libraries(server-tests PRIVATE
  gtest
  gtest_main
  shared-server)

add_test(
  NAME server-tests
  COMMAND server-tests)
