# NOTE: Unused - no CMake client builds currently.
add_executable(client-tests
  IntersectTriangle_test.cpp
  Matrix44_test.cpp
  Vector2_test.cpp
  Vector3_test.cpp
  Vector4_test.cpp)

target_link_libraries(client-tests PRIVATE
  gtest
  gtest_main)

add_test(
  NAME ClientTests
  COMMAND client-tests)
