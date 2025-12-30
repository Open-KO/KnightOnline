# Get dx9sdk package
#
# Makes the dx9sdk target available.

include(FetchContent)

fetchcontent_declare(
  dx9sdk
  GIT_REPOSITORY        "https://github.com/Open-KO/microsoft-directx-sdk.git"
  GIT_TAG               "v9.0-OpenKO"
  GIT_PROGRESS          ON
  GIT_SHALLOW           ON
  SOURCE_DIR            "${FETCHCONTENT_BASE_DIR}/dx9sdk"

  EXCLUDE_FROM_ALL
)

fetchcontent_makeavailable(dx9sdk)

set(DX9_INCLUDE_DIR "${dx9sdk_SOURCE_DIR}/Include")
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(DX9_LIBRARY_DIR "${dx9sdk_SOURCE_DIR}/Lib/x64")
else()
  set(DX9_LIBRARY_DIR "${dx9sdk_SOURCE_DIR}/Lib/x86")
endif()

add_library(dx9sdk INTERFACE
  "${DX9_INCLUDE_DIR}/d3d9.h"
  "${DX9_INCLUDE_DIR}/d3d9caps.h"
  "${DX9_INCLUDE_DIR}/d3d9types.h"
  "${DX9_INCLUDE_DIR}/d3dx9.h"
  "${DX9_INCLUDE_DIR}/d3dx9anim.h"
  "${DX9_INCLUDE_DIR}/d3dx9core.h"
  "${DX9_INCLUDE_DIR}/d3dx9effect.h"
  "${DX9_INCLUDE_DIR}/d3dx9math.h"
  "${DX9_INCLUDE_DIR}/d3dx9math.inl"
  "${DX9_INCLUDE_DIR}/d3dx9mesh.h"
  "${DX9_INCLUDE_DIR}/d3dx9shader.h"
  "${DX9_INCLUDE_DIR}/d3dx9shape.h"
  "${DX9_INCLUDE_DIR}/d3dx9tex.h"
  "${DX9_INCLUDE_DIR}/d3dx9xof.h"
  "${DX9_INCLUDE_DIR}/dinput.h"
  "${DX9_INCLUDE_DIR}/dinputd.h"
  "${DX9_INCLUDE_DIR}/dsetup.h"
  "${DX9_INCLUDE_DIR}/dsound.h"
  "${DX9_INCLUDE_DIR}/dxdiag.h"
  "${DX9_INCLUDE_DIR}/DxErr.h"
  "${DX9_INCLUDE_DIR}/dxsdkver.h"
)

target_compile_definitions(dx9sdk INTERFACE
  "USE_DIRECTX9=1"
  "DIRECTINPUT_VERSION=0x0800"
)

# Expose include path
target_include_directories(dx9sdk INTERFACE
  "${DX9_INCLUDE_DIR}"
)

# Link in the libs that we use
target_link_libraries(dx9sdk INTERFACE
  "${DX9_LIBRARY_DIR}/d3d9.lib"
  "${DX9_LIBRARY_DIR}/d3dx9.lib"
  "${DX9_LIBRARY_DIR}/dinput8.lib"
  "${DX9_LIBRARY_DIR}/dsound.lib"
  "${DX9_LIBRARY_DIR}/dxerr.lib"
  "${DX9_LIBRARY_DIR}/dxguid.lib"
)

