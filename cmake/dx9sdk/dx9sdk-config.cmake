# Get dx9sdk package
#
# Makes the dx9sdk target available.

include(FetchContent)

fetchcontent_declare(
  dx9sdk
  GIT_REPOSITORY        "https://github.com/Open-KO/microsoft-directx-sdk.git"
  GIT_TAG               "dx9-june-2010"
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

# Link in the libs that we use
target_link_libraries(dx9sdk INTERFACE
  "${DX9_LIBRARY_DIR}/d3d9.lib"
  "${DX9_LIBRARY_DIR}/d3dx9.lib"
  "${DX9_LIBRARY_DIR}/dinput8.lib"
  "${DX9_LIBRARY_DIR}/dsound.lib"
  "${DX9_LIBRARY_DIR}/dxerr.lib"
  "${DX9_LIBRARY_DIR}/dxguid.lib"
)

# We need a very explicit include order for DX9, because of its awkward header conflicts.
#
# To ensure this, we must append it to IncludePath (not AdditionalIncludeDirectories).
# This is technically exposed via VS_GLOBAL_IncludePath, but it's inserted before the
# VS defaults are actually added, which would prepend rather than append if VS didn't
# outright not bother to set it up because it has a value already.
#
# Instead, we can use a property sheet. This replaces the VS user default prop sheet
# (which we must reimport ourselves), but is added late enough that we can append to
# the existing IncludePath setting and ensure our path is consistent.
#
# The only problem with this approach is that we cannot propagate this to consumers.
# So we add require_directx9() to force consumers to give themselves the property sheet
# manually.
set(DIRECTX9_PROPS_TEMPLATE "${OPENKO_MODULE_PATH}/dx9sdk/directx9.props.template")
set(DIRECTX9_PROPS_DIR "${CMAKE_CURRENT_BINARY_DIR}/props")
set(DIRECTX9_PROPS_PATH "${DIRECTX9_PROPS_DIR}/directx9.props" CACHE STRING "DirectX 9 property sheet path for includes (internal)" FORCE)

# Make sure the target directory exists
file(MAKE_DIRECTORY "${DIRECTX9_PROPS_DIR}")

configure_file("${DIRECTX9_PROPS_TEMPLATE}" "${DIRECTX9_PROPS_PATH}" @ONLY)

function(require_directx9 TARGET)
  if(NOT TARGET ${TARGET})
    message(FATAL_ERROR "Target ${TARGET} does not exist!")
  endif()

  if(MSVC)
    set_target_properties(${TARGET} PROPERTIES VS_USER_PROPS "${DIRECTX9_PROPS_PATH}")
  endif()
endfunction()
