add_executable(MathUtils.Tests
  CollisionBox_test.cpp
  Convert2DTo3DCoordinate_test.cpp
  Convert3DTo2DCoordinate_test.cpp
  IntersectTriangle_test.cpp
  Matrix44_test.cpp
  Quaternion_test.cpp
  Vector2_test.cpp
  Vector3_test.cpp
  Vector4_test.cpp
  Yaw2D_test.cpp)

target_link_libraries(MathUtils.Tests PRIVATE
  gtest
  gtest_main
  MathUtils)

add_test(
  NAME MathUtilsTests
  COMMAND MathUtils.Tests)
