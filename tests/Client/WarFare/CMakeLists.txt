add_executable(WarFare.Network.Tests
  APISocketPayload_test.cpp)

set_target_properties(WarFare.Network.Tests PROPERTIES
  CXX_SCAN_FOR_MODULES OFF
)

target_include_directories(WarFare.Network.Tests PRIVATE
  "${CMAKE_SOURCE_DIR}/src")

target_link_libraries(WarFare.Network.Tests PRIVATE
  gtest
  gtest_main)

add_test(
  NAME WarFareNetworkTests
  COMMAND WarFare.Network.Tests)
