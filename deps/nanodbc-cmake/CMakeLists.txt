fetchcontent_declare(
  nanodbc
  GIT_REPOSITORY        "https://github.com/Open-KO/nanodbc.git"
  GIT_TAG               "v2.1.4.0a-OpenKO"
  GIT_PROGRESS          ON
  GIT_SHALLOW           ON
  OVERRIDE_FIND_PACKAGE TRUE
  EXCLUDE_FROM_ALL
)

set(NANODBC_FORCE_LIBCXX OFF CACHE BOOL "Force the use of libc++ (default off)")
set(NANODBC_DISABLE_MSSQL_TVP ON CACHE BOOL "Disable MSSQL Table-valued parameters in nanodbc")
set(NANODBC_DISABLE_NULL_ACCESS_ERROR ON CACHE BOOL "Disable null_access_error() exceptions in nanodbc")
set(NANODBC_USE_UINT8_FOR_TINYINT ON CACHE BOOL "Enable use of uint8 to represent TINYINT fields instead of int16 in nanodbc")

message(STATUS "OpenKO: [nanodbc] Checking and fetching...")

fetchcontent_makeavailable(nanodbc)

message(STATUS "OpenKO: [nanodbc] Up-to-date!")

# Use C++20 explicitly for better compatibility, but it needs updates so we don't want to push our luck
target_compile_features(nanodbc PUBLIC cxx_std_20)

# codecvt conversions are being deprecated.
# For now we'll just ignore the warning and hope they fix it before it's removed.
# Note that nanodbc does support Boost.Locale as an alternative, so we could use
# that if we had to.
target_compile_options(nanodbc
  PRIVATE
  $<$<CXX_COMPILER_ID:Clang>:-Wno-deprecated-declarations>
)
