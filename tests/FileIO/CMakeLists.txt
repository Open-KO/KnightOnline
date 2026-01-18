add_executable(FileIO.Tests
  FileReader_test.cpp
  FileReaderSetup_test.cpp
  FileWriter_test.cpp
  FileWriterSetup_test.cpp)

set_target_properties(FileIO.Tests PROPERTIES
  CXX_SCAN_FOR_MODULES OFF
)

target_link_libraries(FileIO.Tests PRIVATE
  gtest
  gtest_main
  FileIO)

add_test(
  NAME FileIOTests
  COMMAND FileIO.Tests)
