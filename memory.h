#pragma once

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

#ifndef BF_MEMCAP_MAX
# define BF_MEMCAP_MAX (4 * 1024)
#endif // BF_MEMCAP_MAX

typedef char bf_mem_data_t;

/**
 * @brief Memory.
 */
typedef struct __bf_memory
{
    bf_mem_data_t * current; ///< pointer to current cell

    bf_mem_data_t * _data;
    bf_mem_data_t * _data_last;
} bf_memory_t;

/**
 * @brief Initialize a bf_memory struct.
 *
 * @param mem pointer to the bf_memory to initialize
 * @param capacity initial capacity, must be greater than 15
 *
 * @return On success, returns 0; on error, returns a negative number.
 */
_BF_INLINE int bf_mem_initialize(bf_memory_t * mem, size_t capacity)
{
    if (capacity < 16)
        return -1;
    const size_t n_bytes = capacity * sizeof(bf_mem_data_t);

    void * ptr = malloc(n_bytes);
    if (ptr == NULL)
        return -2;

    memset(ptr, 0, n_bytes);

    mem->_data      = (bf_mem_data_t *)ptr;
    mem->_data_last = (bf_mem_data_t *)ptr + capacity - 1;
    mem->current    = mem->_data + capacity / 2;

    return 0;
}

/**
 * @brief Finalize a bf_memory struct.
 *
 * @param mem pointer to the bf_memory to finalize
 */
_BF_INLINE void bf_mem_finalize(bf_memory_t * mem)
{
    if (mem->_data == NULL)
        return;

    free(mem->_data);
    mem->_data = NULL;
}

_BF_INLINE void bf_mem_dump(const bf_memory_t * mem)
{
    bf_mem_data_t * p = mem->_data;

    printf("bf_memory@%p: {size=%u,data=[%d",
        mem, mem->_data_last - mem->_data + 1, *p++);

    for (; p < mem->_data_last; p++)
        printf(",%d", *p);

    printf("]}\n");
}

// Expand forward
_BF_INLINE void __bf_mem_e_f(bf_memory_t * mem)
{
    BF_DEBUG_LOG("calling: %s(%p)", __func__, mem);

    const size_t old_cap = mem->_data_last - mem->_data + 1;
    const size_t new_cap = old_cap + old_cap / 2;
    const size_t cur_off = mem->current - mem->_data;

    const size_t app_cap_bytes = (old_cap / 2) * sizeof(bf_mem_data_t);
    const size_t new_cap_bytes = new_cap * sizeof(bf_mem_data_t);
    const size_t cur_off_bytes = cur_off * sizeof(bf_mem_data_t);

    if (new_cap > BF_MEMCAP_MAX)
    {
        bf_print_message(2, "out of memory");
        exit(1);
    }

    void * ptr = malloc(new_cap_bytes);
    if (ptr == NULL)
    {
        bf_print_message(2, "memory error");
        exit(1);
    }

    memset(ptr, 0, app_cap_bytes);
    memcpy(ptr + app_cap_bytes, mem->_data, old_cap * sizeof(bf_mem_data_t));

    free(mem->_data);

    mem->_data      = (bf_mem_data_t *)ptr;
    mem->_data_last = (bf_mem_data_t *)ptr + new_cap - 1;
    mem->current    = mem->_data + cur_off + old_cap / 2;

    BF_DEBUG_LOG("bf_memory %p: current capacity: %u", mem, new_cap);
}

/**
 * @brief Move forward.
 *
 * @param mem pointer to a bf_memory
 */
_BF_INLINE void bf_mem_movef(bf_memory_t * mem)
{
    if (mem->current <= mem->_data)
        __bf_mem_e_f(mem);

    mem->current--;
}

// Expand backword
_BF_INLINE void __bf_mem_e_b(bf_memory_t * mem)
{
    BF_DEBUG_LOG("calling: %s(%p)", __func__, mem);

    const size_t old_cap = mem->_data_last - mem->_data + 1;
    const size_t new_cap = old_cap + old_cap / 2;
    const size_t cur_off = mem->current - mem->_data;

    const size_t odl_cap_bytes = old_cap * sizeof(bf_mem_data_t);
    const size_t new_cap_bytes = new_cap * sizeof(bf_mem_data_t);
    const size_t cur_off_bytes = cur_off * sizeof(bf_mem_data_t);

    if (new_cap > BF_MEMCAP_MAX)
    {
        bf_print_message(2, "out of memory");
        exit(1);
    }

    void * ptr = malloc(new_cap_bytes);
    if (ptr == NULL)
    {
        bf_print_message(2, "memory error");
        exit(1);
    }

    memcpy(ptr, mem->_data, odl_cap_bytes);
    memset(ptr + odl_cap_bytes, 0, (old_cap / 2) * sizeof(bf_mem_data_t));

    free(mem->_data);

    mem->_data      = (bf_mem_data_t *)ptr;
    mem->_data_last = (bf_mem_data_t *)ptr + new_cap - 1;
    mem->current    = mem->_data + cur_off;

    BF_DEBUG_LOG("bf_memory %p: current capacity: %u", mem, new_cap);
}

/**
 * @brief Move backward.
 *
 * @param mem pointer to a bf_memory
 */
_BF_INLINE void bf_mem_moveb(bf_memory_t * mem)
{
    if (mem->current >= mem->_data_last)
        __bf_mem_e_b(mem);

    mem->current++;
}

/**
 * @brief Increase current data.
 *
 * @param mem pointer to a bf_memory
 */
_BF_INLINE void bf_mem_dinc(bf_memory_t * mem)
{
    (*mem->current)++;
}

/**
 * @brief Decrease current data.
 *
 * @param mem pointer to a bf_memory
 */
_BF_INLINE void bf_mem_ddec(bf_memory_t * mem)
{
    (*mem->current)--;
}

/**
 * @brief Read current data.
 *
 * @param mem pointer to a bf_memory
 *
 * @return Current data.
 */
_BF_INLINE bf_mem_data_t bf_mem_read(bf_memory_t * mem)
{
    return *mem->current;
}

/**
 * @brief Write current data.
 *
 * @param mem pointer to a bf_memory
 * @param val new value
 *
 * @return Current data.
 */
_BF_INLINE void bf_mem_write(bf_memory_t * mem, bf_mem_data_t val)
{
    *mem->current = val;
}
