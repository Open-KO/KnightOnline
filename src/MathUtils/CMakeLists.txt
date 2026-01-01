add_library(MathUtils STATIC
  GeometricStructs.h
  MathUtils.cpp
  MathUtils.h
  Matrix44.h
  Matrix44.inl
  Quaternion.h
  Quaternion.inl
  Vector2.h
  Vector2.inl
  Vector3.h
  Vector3.inl
  Vector4.h
  Vector4.inl)

# Enable warnings as errors
if(OPENKO_COMPILE_WARNINGS_AS_ERROR)
  set_property(TARGET MathUtils PROPERTY COMPILE_WARNING_AS_ERROR ON)
endif()

# Expose include path
target_include_directories(MathUtils PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}/../" # so we can access <MathUtils/...>
)

# Setup warning levels
target_compile_options(MathUtils PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W3>
  $<$<CXX_COMPILER_ID:GNU>:-Wall>
  $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra>
)
