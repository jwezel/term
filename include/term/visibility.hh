#pragma once
#if defined _WIN32 || defined __CYGWIN__
  #ifdef BUILDING_TERM
    #define TERM_PUBLIC __declspec(dllexport)
  #else
    #define TERM_PUBLIC __declspec(dllimport)
  #endif
#else
  #ifdef BUILDING_TERM
      #define TERM_PUBLIC __attribute__ ((visibility ("default")))
  #else
      #define TERM_PUBLIC
  #endif
#endif
