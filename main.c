#include <stdio.h>

#include "iostream.h"
#include "memory.h"
#include "program.h"
#include "util.h"

static int compile(bf_program_t * program, const char * filename)
{
    if (filename == NULL)
    {
        return bf_prog_compile(program, stdin);
    }
    else
    {
        FILE * f = fopen(filename, "r");
        if (f == NULL)
        {
            bf_print_message(2, "cannot open file %s", filename);
            return -1;
        }

        int r = bf_prog_compile(program, f);
        fclose(f);
        return r;
    }
}

static void execute(bf_program_t * program, bf_memory_t * memory)
{
    const char * lb = NULL, * rb = NULL; // '[', ']'

    while (1)
    {
        char instr = bf_prog_get(program);

        switch (instr)
        {
        case _BF_i_halt:
            goto _END_OF_LOOP;

        case _BF_i_next:
            bf_mem_moveb(memory);
            break;

        case _BF_i_prev:
            bf_mem_movef(memory);
            break;

        case _BF_i_inc:
            bf_mem_dinc(memory);
            break;

        case _BF_i_dec:
            bf_mem_ddec(memory);
            break;

        case _BF_i_out:
            bf_io_output(bf_mem_read(memory));
            break;

        case _BF_i_in:
            bf_mem_write(memory, bf_io_input());
            break;

        case _BF_i_jbz:
            lb = bf_prog_pos(program);
            if (!bf_mem_read(memory))
            {
                if (rb != NULL)
                {
                    bf_prog_jump(program, rb + 1);
                    rb = NULL;
                }
                else
                {
                    while (1)
                    {
                        char x = bf_prog_get(program);
                        if (x == _BF_i_jfnz)
                            break;
                        else if (x == _BF_i_halt)
                            bf_print_message(2, "unexpected '[' instruction, "
                                "can't find a previous ']'");
                    }
                }
            }
            break;

        case _BF_i_jfnz:
            if (bf_mem_read(memory))
            {
                if (lb != NULL)
                {
                    bf_prog_jump(program, lb + 1);
                }
                else
                {
                    bf_print_message(2,
                        "unexpected ']' instruction, can't find a previous '['");
                }
            }
            break;

        default:
            bf_print_message(2, "unexpected instruction 0x%02x", instr);
            exit(2);
        }
    }
    _END_OF_LOOP:;
}

int main(int argc, const char * argv[])
{
    bf_memory_t     memory;
    bf_program_t    program;

    if (argc != 2 && argc != 1)
        return 1;

    if (bf_mem_initialize(&memory, 32) || bf_prog_initialize(&program))
        return 1;

    if (compile(&program, argc == 2 ? argv[1] : NULL))
    {
        bf_mem_finalize(&memory);
        bf_prog_finalize(&program);
        return 1;
    }

    execute(&program, &memory);

    bf_mem_finalize(&memory);
    bf_prog_finalize(&program);

    return 0;
}
