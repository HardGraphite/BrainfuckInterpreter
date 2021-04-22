#pragma once

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

/**
 * @brief Instructions.
 */
typedef enum __bf_instr
{
    bf_i_halt, ///< stop
    bf_i_next, ///< next progory cell, `>`
    bf_i_prev, ///< previous progory cell, `<`
    bf_i_inc , ///< increase, `+`
    bf_i_dec , ///< decrease, `-`
    bf_i_out , ///< output, `.`
    bf_i_in  , ///< input, `,`
    bf_i_jbz , ///< jump to next ']' if zero, `[`
    bf_i_jfnz, ///< jump to previous '[' if non-zero, `]`
} bf_instr_t;

#define _BF_i_halt   ((char)bf_i_halt)
#define _BF_i_next   ((char)bf_i_next)
#define _BF_i_prev   ((char)bf_i_prev)
#define _BF_i_inc    ((char)bf_i_inc )
#define _BF_i_dec    ((char)bf_i_dec )
#define _BF_i_out    ((char)bf_i_out )
#define _BF_i_in     ((char)bf_i_in  )
#define _BF_i_jbz    ((char)bf_i_jbz )
#define _BF_i_jfnz   ((char)bf_i_jfnz)

/**
 * @brief Program.
 */
typedef struct __bf_program
{
    const char  * current; ///< pointer to current instruction

    char        * _code;
    size_t        _code_size;
    char        * _code_end;
} bf_program_t;

/**
 * @brief Initialize a bf_progory struct.
 *
 * @param mem pointer to the bf_progory to initialize
 *
 * @return On success, returns 0; on error, returns a negative number.
 */
_BF_INLINE int bf_prog_initialize(bf_program_t * prog)
{
    const size_t n_bytes = 64;
    void * ptr = malloc(n_bytes);
    if (ptr == NULL)
        return -1;

    prog->current    = (char *)ptr;
    prog->_code      = (char *)ptr;
    prog->_code_size = n_bytes;
    prog->_code_end  = (char *)ptr;

    return 0;
}

/**
 * @brief Finalize a bf_progory struct.
 *
 * @param prog pointer to the bf_progory to finalize
 */
_BF_INLINE void bf_prog_finalize(bf_program_t * prog)
{
    if (prog->_code == NULL)
        return;

    free(prog->_code);
    prog->_code = NULL;
}

/**
 * @brief Compile program from source code.
 *
 * @param prog pointer to the bf_progory to write code to
 * @param source_code stream to read source code from
 *
 * @return On success, returns 0; on error, returns a negative number.
 */
_BF_INLINE int bf_prog_compile(bf_program_t * prog, FILE * source_code)
{
    prog->_code_end = prog->_code;

    size_t left_free_size = prog->_code_size;

    while (1)
    {
        char ch = fgetc(source_code);

        if (!left_free_size)
        {
            BF_DEBUG_LOG("expanding capacity of bf_program %p...", prog);

            const size_t n_bytes = prog->_code_size + prog->_code_size / 2;

            void * ptr = malloc(n_bytes);
            if (ptr == NULL)
                return -1;
            else
                memcpy(ptr, prog->_code, prog->_code_size);

            size_t current_offset = prog->_code_end - prog->_code;

            free(prog->_code);

            prog->_code      = (char *)ptr;
            prog->_code_size = n_bytes;
            prog->_code_end = (char *)ptr + current_offset;

            left_free_size = n_bytes - current_offset;

            BF_DEBUG_LOG("bf_program %p: current capacity: %u", prog, n_bytes);
        }

        switch (ch)
        {
            case (char)EOF:
                *(prog->_code_end++) = _BF_i_halt;
                goto _END_OF_LOOP;

            case '\t':
            case '\n':
            case ' ':
                break;

#define _CASE(CH, I) case CH : *(prog->_code_end++) = I; break

            _CASE('+', _BF_i_inc );
            _CASE(',', _BF_i_in  );
            _CASE('-', _BF_i_dec );
            _CASE('.', _BF_i_out );
            _CASE('<', _BF_i_prev);
            _CASE('>', _BF_i_next);
            _CASE('[', _BF_i_jbz );
            _CASE(']', _BF_i_jfnz);

#undef _CASE

            default:
                if (ch == '/')
                {
                    ch = fgetc(source_code);
                    if (ch == '/')
                        while (1)
                        {
                            ch = fgetc(source_code);
                            if (ch == EOF || ch == '\n')
                                break;
                        }
                    else
                        ; // bf_print_message(1, "unexpected characters '/%c'", ch);
                }
                else
                {
                    // bf_print_message(1, "unexpected character '%c'", ch);
                }
                break;
        }

        left_free_size--;
    }
    _END_OF_LOOP:

    prog->current = prog->_code;

    return 0;
}

/**
 * @brief Get next instruction.
 *
 * @param prog pointer to a bf_progory
 *
 * @return Next instruction.
 */
_BF_INLINE char bf_prog_get(bf_program_t * prog)
{
    return *(prog->current++);
}

/**
 * @brief Get get current position.
 *
 * @param prog pointer to a bf_progory
 *
 * @return Position.
 */
_BF_INLINE const char * bf_prog_pos(bf_program_t * prog)
{
    return prog->current - 1;
}

/**
 * @brief Jump.
 *
 * @param prog pointer to a bf_progory
 * @param p where to jump to
 */
_BF_INLINE void bf_prog_jump(bf_program_t * prog, const char * p)
{
    prog->current = p;
}

/**
 * @brief Jump to previous `bf_i_jbz`.
 *
 * @param prog pointer to a bf_progory
 */
_BF_INLINE void bf_prog_jumpf(bf_program_t * prog)
{
    const char * const begin = prog->_code;
    const char *       ip    = prog->current;

    int counter = 0;

    while (1)
    {
        if (ip < begin)
        {
            bf_print_message(2, "can't find previous '['");
            exit(1);
        }

        switch (*ip)
        {
        case _BF_i_jbz:
            counter++;
            if (counter >= 0)
            {
                prog->current = ip + 1;
                return;
            }
            break;

        case _BF_i_jfnz:
            counter--;
            break;

        default:
            break;
        }

        ip--;
    }
}

/**
 * @brief Jump to next `bf_i_jfnz`.
 *
 * @param prog pointer to a bf_progory
 */
_BF_INLINE void bf_prog_jumpb(bf_program_t * prog)
{
    const char * const begin = prog->_code;
    const char *       ip    = prog->current;

    int counter = 0;

    while (1)
    {
        if (ip < begin)
        {
            _NOT_FOUND:
            bf_print_message(2, "can't find next ']'");
            exit(1);
        }

        switch (*ip)
        {
        case _BF_i_halt:
            goto _NOT_FOUND;

        case _BF_i_jbz:
            counter++;
            break;

        case _BF_i_jfnz:
            counter--;
            if (counter <= 0)
            {
                prog->current = ip + 1;
                return;
            }
            break;

        default:
            break;
        }

        ip++;
    }
}
