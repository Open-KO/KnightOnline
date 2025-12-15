# NOTE: Unused - no CMake client builds currently.
add_executable(client-tests
  CollisionBox_test.cpp
  Convert2DTo3DCoordinate_test.cpp
  Convert3DTo2DCoordinate_test.cpp
  IntersectTriangle_test.cpp
  Matrix44_test.cpp
  Vector2_test.cpp
  Vector3_test.cpp
  Vector4_test.cpp
  Yaw2D_test.cpp)

target_link_libraries(client-tests PRIVATE
  gtest
  gtest_main)

add_test(
  NAME ClientTests
  COMMAND client-tests)
