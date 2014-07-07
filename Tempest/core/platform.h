#ifndef PLATFORM_H
#define PLATFORM_H

#if defined(WINAPI_FAMILY) && (WINAPI_FAMILY==WINAPI_FAMILY_PHONE_APP)
  #undef  __WINDOWS_PHONE__
  #define __WINDOWS_PHONE__ 1
#endif

#if (defined(WIN32) || defined(_WIN32)) && !defined(__WINDOWS_PHONE__)
  #undef  __WINDOWS__
  #define __WINDOWS__	1
#endif

#if defined(linux) || defined(__linux) || defined(__linux__)
  #ifndef __ANDROID__
    #undef  __LINUX__
    #define __LINUX__	1
  #else
    #undef  __ANDROID__
    #define __ANDROID__	1
  #endif
#endif

#endif // PLATFORM_H
