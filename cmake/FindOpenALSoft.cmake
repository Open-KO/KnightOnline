# Get OpenALSoft package
#
# Makes the openal-soft target available.

fetchcontent_declare(
  openalsoft
  GIT_REPOSITORY        "https://github.com/Open-KO/openal-soft.git"
  GIT_TAG               "1.25.0-32f75f"
  GIT_PROGRESS          ON
  GIT_SHALLOW           ON
  EXCLUDE_FROM_ALL
)

set(ALSOFT_UTILS OFF CACHE BOOL "openal-soft: Build utility programs")
set(ALSOFT_EXAMPLES OFF CACHE BOOL "openal-soft: Build example programs")
set(ALSOFT_ENABLE_MODULES OFF CACHE BOOL "openal-soft: Enable use of C++20 modules when supported")

if(WIN32)
  set(FORCE_STATIC_VCRT ON CACHE BOOL "openal-soft: Force /MT for static VC runtimes")
  set(LIBTYPE STATIC CACHE STRING "openal-soft: Library type")
endif()

# We expect to run on Windows
# Turn off all backends that don't support Windows.
if(MINGW)
  set(ALSOFT_BACKEND_PIPEWIRE OFF CACHE BOOL "openal-soft: Enable PipeWire backend" FORCE)
  set(ALSOFT_BACKEND_PULSEAUDIO OFF CACHE BOOL "openal-soft: Enable PulseAudio backend" FORCE)
  set(ALSOFT_BACKEND_ALSA OFF CACHE BOOL "openal-soft: Enable ALSA backend" FORCE)
  set(ALSOFT_BACKEND_OSS OFF CACHE BOOL "openal-soft: Enable OSS backend" FORCE)
  set(ALSOFT_BACKEND_SOLARIS OFF CACHE BOOL "openal-soft: Enable Solaris backend" FORCE)
  set(ALSOFT_BACKEND_SNDIO OFF CACHE BOOL "openal-soft: Enable SndIO backend" FORCE)
  set(ALSOFT_BACKEND_JACK OFF CACHE BOOL "openal-soft: Enable JACK backend" FORCE)
  set(ALSOFT_BACKEND_COREAUDIO OFF CACHE BOOL "openal-soft: Enable CoreAudio backend" FORCE)
  set(ALSOFT_BACKEND_OBOE OFF CACHE BOOL "openal-soft: Enable Oboe backend" FORCE)
  set(ALSOFT_BACKEND_OPENSL OFF CACHE BOOL "openal-soft: Enable OpenSL backend" FORCE)
  # set(ALSOFT_BACKEND_PORTAUDIO OFF CACHE BOOL "openal-soft: Enable PortAudio backend" FORCE)
  # set(ALSOFT_BACKEND_WAVE OFF CACHE BOOL "openal-soft: Enable Wave Writer backend" FORCE)
endif()

fetchcontent_makeavailable(openalsoft)

set(openalsoft_FOUND TRUE)
