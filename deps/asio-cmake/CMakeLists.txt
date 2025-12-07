set(ASIO_DIR "${OPENKO_DEPS_DIR}/asio/asio")

add_library(asio STATIC
  ${ASIO_DIR}/src/asio.cpp
  ${ASIO_DIR}/include/asio.hpp
)

# Use C++23
target_compile_features(asio PUBLIC cxx_std_23)

# Build as standalone
target_compile_definitions(asio PUBLIC ASIO_STANDALONE)
target_compile_definitions(asio PRIVATE ASIO_SEPARATE_COMPILATION)

target_include_directories(asio
  PUBLIC ${ASIO_DIR}/include
  PRIVATE ${OPENKO_DEPS_DIR}/asio
)
