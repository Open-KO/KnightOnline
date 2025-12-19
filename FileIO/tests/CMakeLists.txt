add_executable(FileIO-tests
  FileReader_test.cpp
  FileReaderSetup_test.cpp
  FileWriter_test.cpp
  FileWriterSetup_test.cpp)

target_link_libraries(FileIO-tests PRIVATE
  gtest
  gtest_main
  FileIO)

add_test(
  NAME FileIOTests
  COMMAND FileIO-tests)
