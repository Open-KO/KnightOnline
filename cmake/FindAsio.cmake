# Get Asio package
#
# Makes the asio target available.

fetchcontent_declare(
  asio
  GIT_REPOSITORY        "https://github.com/chriskohlhoff/asio.git"
  GIT_TAG               "asio-1-36-0"
  GIT_PROGRESS          ON
  GIT_SHALLOW           ON
  OVERRIDE_FIND_PACKAGE TRUE
  EXCLUDE_FROM_ALL
)

fetchcontent_makeavailable(asio)

add_library(asio STATIC
  "${asio_SOURCE_DIR}/asio/src/asio.cpp"
  "${asio_SOURCE_DIR}/asio/include/asio.hpp"
)

target_include_directories(asio
  PUBLIC "${asio_SOURCE_DIR}/asio/include"
  PRIVATE "${asio_SOURCE_DIR}"
)

# Build as standalone
target_compile_definitions(asio PUBLIC ASIO_STANDALONE ASIO_NO_DEPRECATED)
target_compile_definitions(asio PRIVATE ASIO_SEPARATE_COMPILATION)

set(asio_FOUND TRUE)
