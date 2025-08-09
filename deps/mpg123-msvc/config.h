// Define to only include portable API in libraries.
/* #undef PORTABLE_API */

// Define to use proper rounding.
#define ACCURATE_ROUNDING 1

// Define if .balign is present.
/* #undef ASMALIGN_BALIGN */

// Define if .align takes 3 for alignment of 2^3=8 bytes instead of 8.
/* #undef ASMALIGN_EXP */

// Define if .align just takes byte count.
#define ASMALIGN_BYTE 1

// Define if __attribute__((aligned(16))) shall be used
/* #undef CCALIGN */

#define DEFAULT_OUTPUT_MODULE "win32"

/* #undef DEBUG */
/* #undef DYNAMIC_BUILD */

// Define if FIFO support is enabled.
#define FIFO 1

// Define if frame index should be used.
#define FRAME_INDEX 1

#define GAPLESS 1
#define HAVE_ATOLL 1
/* #undef HAVE_DIRENT_H */
/* #undef HAVE_DLFCN_H */
/* #undef HAVE_INTTYPES_H */
/* #undef HAVE_LANGINFO_H */
#define HAVE_LOCALE_H 1
/* #undef HAVE_NL_LANGINFO */
/* #undef HAVE_RANDOM */
/* #undef HAVE_SCHED_H */
#define HAVE_SETLOCALE 1
/* #undef HAVE_USELOCALE */
/* #undef HAVE_SETPRIORITY */
#define HAVE_SIGNAL_H 1
#define HAVE_STRERROR 1
/* #undef HAVE_STRERROR_L */
/* #undef HAVE_STRTOK_R */
#define HAVE_STRTOK_S 1
/* #undef HAVE_FORK */
#define HAVE_EXECVP 1
/* #undef HAVE_CTERMID */
/* #undef HAVE_CLOCK_GETTIME */
/* #undef HAVE_STRINGS_H */
/* #undef HAVE_SYS_IOCTL_H */
/* #undef HAVE_SYS_RESOURCE_H */
/* #undef HAVE_SYS_SELECT_H */
/* #undef HAVE_SYS_SIGNAL_H */
#define HAVE_SYS_STAT_H 1
/* #undef HAVE_SYS_TIME_H */
#define HAVE_SYS_TYPES_H 1
/* #undef HAVE_SYS_WAIT_H */

// Define this if you have the POSIX termios library
/* #undef HAVE_TERMIOS */

/* #undef HAVE_UNISTD_H */
#define HAVE_WINDOWS_H 1

// Define to indicate that float storage follows IEEE754.
#define IEEE_FLOAT 1

#define INDEX_SIZE 1000

// Define if IPV6 support is enabled.
/* #undef IPV6 */

#define LT_MODULE_EXT ".dll"

// Define if network support is enabled.
/* #undef NETWORK */
// Define for new-style network code.
/* #undef NET123 */
/* #undef NET123_EXEC */
/* #undef NET123_WINHTTP */
/* #undef NET123_WININET */

// Define to disable downsampled decoding.
/* #undef NO_DOWNSAMPLE */

// Define to disable equalizer.
/* #undef NO_EQUALIZER */

// Define to disable error messages in combination with a return value (the return is left intact).
/* #undef NO_ERETURN */

// Define to disable error messages.
/* #undef NO_ERRORMSG */

// no feeder decoding, no buffered readers
/* #undef NO_FEEDER */

// Define to disable ICY handling.
/* #undef NO_ICY */

// Define to disable ID3v2 parsing.
/* #undef NO_ID3V2 */

// Define to disable layer I.
/* #undef NO_LAYER1 */

// Define to disable layer II.
/* #undef NO_LAYER2 */

// Define to disable layer III.
/* #undef NO_LAYER3 */

// Define to disable analyzer info.
/* #undef NO_MOREINFO */

// Define to disable ntom resampling.
/* #undef NO_NTOM */

// Define to disable 8 bit integer output.
/* #undef NO_8BIT */

// Define to disable 16 bit integer output.
/* #undef NO_16BIT */

// Define to disable 32 bit and 24 bit integer output.
/* #undef NO_32BIT */

// Define to disable real output.
/* #undef NO_REAL */

// Define to disable string functions.
/* #undef NO_STRING */

// Define for post-processed 32 bit formats.
/* #undef NO_SYNTH32 */

// Define to disable warning messages.
/* #undef NO_WARNING */

#define PACKAGE_NAME "mpg123"
#define PACKAGE_VERSION "1.33.2"

#define PKGLIBDIR "lib/mpg123"

// CMake leaves it emtpy for non-existing type. Autoconf sets it to 0.
#define SIZEOF_OFF_T (4+0)

#define STDERR_FILENO (_fileno(stderr))
#define STDIN_FILENO (_fileno(stdin))
#define STDOUT_FILENO (_fileno(stdout))

// Define to not duplicate some code for likely cases in libsyn123.
/* #undef SYN123_NO_CASES */

/* #undef USE_MODULES */

// Define for new Huffman decoding scheme.
#define USE_NEW_HUFFTABLE 1

// Define to use Unicode for Windows
#define WANT_WIN32_UNICODE 1

#ifdef WANT_WIN32_UNICODE
# define strcasecmp _stricmp
# define strncasecmp _strnicmp
#endif

// Define to use Win32 named pipes
/* #undef WANT_WIN32_FIFO */

/* #undef WORDS_BIGENDIAN */

/* #undef LFS_LARGEFILE_64 */
/* #undef LFS_SENSITIVE */
/* #undef HAVE_O_LARGEFILE */
