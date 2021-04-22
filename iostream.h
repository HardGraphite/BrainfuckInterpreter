#pragma once

#include <stdio.h>

#include "util.h"

#ifndef _WIN32

#include <termios.h>

// stat: 0=reset, 1=echo, 2=noecho
void __bf_io_settermecho(int stat)
{
    static struct termios term_old, term_tmp;
    static _Bool  first_call = 1;

    if (first_call)
    {
        tcgetattr(0, &term_old);
        term_tmp = term_old;
        term_tmp.c_lflag &= ~ICANON;

        first_call = 0;
    }

    if (stat == 0)
        tcsetattr(0, TCSANOW, &term_old);
    else
    {
        if (stat == 1)
            term_tmp.c_lflag |= ECHO;
        else
            term_tmp.c_lflag &= ~ECHO;

        tcsetattr(0, TCSANOW, &term_tmp);
    }
}

int getch(void)
{
    __bf_io_settermecho(2);
    const int c = getchar();
    __bf_io_settermecho(0);

    return c;
}

#endif // _WIN32

_BF_INLINE char bf_io_input(void)
{
    return getch();
}

_BF_INLINE void bf_io_output(char ch)
{
    fputc(ch, stdout);
    fflush(stdout);
}
