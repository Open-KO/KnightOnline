find_package(MFC REQUIRED)

# MFC (static)
set(CMAKE_MFC_FLAG 1)

add_executable(Launcher
  APISocket.cpp
  APISocket.h
  BackgroundUtil.cpp
  BackgroundUtil.h
  BB_CircularBuffer.cpp
  BB_CircularBuffer.h
  Define.h
  Launcher.cpp
  Launcher.h
  LauncherDlg.cpp
  LauncherDlg.h
  MacProgressCtrl.cpp
  MacProgressCtrl.h
  PacketDef.h
  resource.h
  StdAfx.h

  # Resources
  res/Bkg.bmp
  res/Btn_Connect.bmp
  res/Btn_cancel.bmp
  res/Btn_exit.bmp
  res/Btn_join.bmp
  res/Edit_PW.bmp
  res/Edit_id.bmp)

if(WIN32)
  set_target_properties(Launcher PROPERTIES WIN32_EXECUTABLE TRUE)
  target_sources(Launcher PRIVATE
    Launcher.rc
    res/Launcher.rc2
    res/Launcher.ico)
endif()

# Enable warnings as errors
if(OPENKO_COMPILE_WARNINGS_AS_ERROR)
  set_property(TARGET Launcher PROPERTY COMPILE_WARNING_AS_ERROR ON)
endif()

# Setup PCH
target_precompile_headers(Launcher PRIVATE StdAfx.h)

# Setup dependencies
target_link_libraries(Launcher PRIVATE
  ZipArchive
  wininet.lib
)

# Setup preprocessor defines
target_compile_definitions(Launcher PRIVATE
  _CRT_SECURE_NO_WARNINGS
  _WINSOCK_DEPRECATED_NO_WARNINGS)

# Setup warning levels
target_compile_options(Launcher PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W3>
  $<$<CXX_COMPILER_ID:GNU>:-Wall>
  $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra>
)

if(MSVC)
  set_property(TARGET Launcher PROPERTY
    VS_DEBUGGER_WORKING_DIRECTORY "${OPENKO_CLIENT_DIR}"
  )
endif()

add_custom_command(TARGET Launcher POST_BUILD
  COMMAND "${CMAKE_COMMAND}" -E echo "$<TARGET_FILE:Launcher> has been built"
)
