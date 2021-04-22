#pragma once

#include <stdarg.h>
#include <stdio.h>


#define _BF_INLINE static inline


/**
 * @brief Print message.
 *
 * @param level 0=note, 1=warning, 2=error
 * @param format format string
 * @param ... data to format
 */
_BF_INLINE void bf_print_message(int level, const char * format, ...)
{
    const char * l;
    va_list      ap;

    if (level <= 0)
        l = "\x1b[1;36mNOTE:\x1b[0m ";
    else if (level == 1)
        l = "\x1b[1;33mWARNING:\x1b[0m ";
    else if (level >= 2)
        l = "\x1b[1;31mERROR:\x1b[0m ";

    fputs(l, stderr);
    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);
    fputc('\n', stderr);
}


#ifdef _NDEBUG
# define BF_DEBUG_LOG(...) ((void)0)
#else // _NDEBUG
# define BF_DEBUG_LOG(...) bf_print_message(0, __VA_ARGS__)
#endif // _NDEBUG
