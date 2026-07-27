/*
 * mango/kal/kal_platform.h
 *
 * Kernel Abstraction Layer -- platform detection and configuration.
 *
 * This header detects the host operating system and defines
 * feature flags that the rest of the KAL uses to select the
 * correct backend.  To add a new platform, add detection logic
 * here and create the corresponding kal_<platform>.c backend.
 */

#ifndef MANGO_KAL_PLATFORM_H
#define MANGO_KAL_PLATFORM_H

/* -----------------------------------------------------------------------
 *  Platform detection
 *
 *  Exactly one KAL_PLATFORM_* macro will be defined.  If no platform
 *  can be detected, the build will fail with a clear error.
 * ----------------------------------------------------------------------- */

#if defined(__linux__)
    #define KAL_PLATFORM_LINUX    1
    #define KAL_PLATFORM_POSIX    1
    #define KAL_PLATFORM_NAME     "linux"
#elif defined(__APPLE__) && defined(__MACH__)
    #define KAL_PLATFORM_DARWIN   1
    #define KAL_PLATFORM_POSIX    1
    #define KAL_PLATFORM_NAME     "darwin"
#elif defined(__FreeBSD__)
    #define KAL_PLATFORM_FREEBSD  1
    #define KAL_PLATFORM_POSIX    1
    #define KAL_PLATFORM_NAME     "freebsd"
#elif defined(__NetBSD__)
    #define KAL_PLATFORM_NETBSD   1
    #define KAL_PLATFORM_POSIX    1
    #define KAL_PLATFORM_NAME     "netbsd"
#elif defined(__OpenBSD__)
    #define KAL_PLATFORM_OPENBSD  1
    #define KAL_PLATFORM_POSIX    1
    #define KAL_PLATFORM_NAME     "openbsd"
#elif defined(_WIN32) || defined(_WIN64)
    #define KAL_PLATFORM_WINDOWS  1
    #define KAL_PLATFORM_NAME     "windows"
#elif defined(__unix__) || defined(__unix)
    #define KAL_PLATFORM_POSIX    1
    #define KAL_PLATFORM_NAME     "posix"
#else
    #error "KAL: unsupported platform -- no KAL_PLATFORM_* defined"
#endif

/* -----------------------------------------------------------------------
 *  Feature flags
 *
 *  These control optional KAL capabilities.  Not every platform
 *  supports every feature.  A missing feature does not prevent
 *  compilation; it only means the corresponding API is unavailable.
 * ----------------------------------------------------------------------- */

#ifdef KAL_PLATFORM_POSIX
    #define KAL_HAS_FORK         1
    #define KAL_HAS_EXEC         1
    #define KAL_HAS_SOCKETPAIR   1
    #define KAL_HAS_POLL         1
    #define KAL_HAS_PTHREAD      1
    #define KAL_HAS_TERMIOS      1
    #define KAL_HAS_CLOCK_MONO   1
    #define KAL_HAS_MKSTEMP      1
    #define KAL_HAS_ENVIRON      1
#endif

#ifdef KAL_PLATFORM_WINDOWS
    #define KAL_HAS_FORK         0
    #define KAL_HAS_EXEC         0
    #define KAL_HAS_SOCKETPAIR   0
    #define KAL_HAS_POLL         0
    #define KAL_HAS_PTHREAD      0
    #define KAL_HAS_TERMIOS      0
    #define KAL_HAS_CLOCK_MONO   0
    #define KAL_HAS_MKSTEMP      0
    #define KAL_HAS_ENVIRON      1
#endif

/* -----------------------------------------------------------------------
 *  KAL name / version
 * ----------------------------------------------------------------------- */

#define KAL_VERSION_MAJOR   1
#define KAL_VERSION_MINOR   0
#define KAL_VERSION_PATCH   0
#define KAL_VERSION_STRING  "1.0.0"

#endif /* MANGO_KAL_PLATFORM_H */
