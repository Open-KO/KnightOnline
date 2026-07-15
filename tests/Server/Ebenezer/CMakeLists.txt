add_executable(Ebenezer.Tests
  BotCapacity_test.cpp
  BotRegistry_test.cpp
  EventLogic_test.cpp
  ItemExchange_test.cpp
  ItemUpgrade_test.cpp
  RankStipendReward_test.cpp
)

target_link_libraries(Ebenezer.Tests PRIVATE
  gtest
  gtest_main
  Ebenezer.Core)

add_test(
  NAME EbenezerTests
  COMMAND Ebenezer.Tests)
