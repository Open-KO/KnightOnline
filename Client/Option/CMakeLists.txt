find_package(MFC REQUIRED)

# MFC (static)
set(CMAKE_MFC_FLAG 1)

add_executable(Option
  Option.cpp
  Option.h
  OptionDlg.cpp
  OptionDlg.h
  resource.h
  StdAfx.h)

if(WIN32)
  set_target_properties(Option PROPERTIES WIN32_EXECUTABLE TRUE)
  target_sources(Option PRIVATE Option.rc)
  target_compile_definitions(Option PRIVATE
    "_UNICODE"
    "UNICODE"
  )
endif()

# Enable warnings as errors
if(OPENKO_COMPILE_WARNINGS_AS_ERROR)
  set_property(TARGET Option PROPERTY COMPILE_WARNING_AS_ERROR ON)
endif()

# Setup PCH
target_precompile_headers(Option PRIVATE StdAfx.h)

# Setup warning levels
target_compile_options(Option PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W3>
  $<$<CXX_COMPILER_ID:GNU>:-Wall>
  $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra>
)

if(MSVC)
  set_property(TARGET Option PROPERTY
    VS_DEBUGGER_WORKING_DIRECTORY "${OPENKO_CLIENT_DIR}"
  )
endif()

add_custom_command(TARGET Option POST_BUILD
  COMMAND "${CMAKE_COMMAND}" -E echo "$<TARGET_FILE:Option> has been built"
)
