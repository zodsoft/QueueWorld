/*
    Queue World is copyright (c) 2014 Ross Bencina

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/
#ifndef INCLUDED_QW_ATOMIC_H
#define INCLUDED_QW_ATOMIC_H

#include "mintomic/mintomic.h"

/*
    Additional atomic functions:

        qw_mint_exchange_32_relaxed
        qw_mint_exchange_64_relaxed
        qw_mint_exchange_ptr_relaxed

    Mintomic lacks atomic swap. I've posted an issue about this:
        https://github.com/mintomic/mintomic/issues/7

    For now we provide our own versions here. Perhaps we'll patch Mintomic later.
*/

#if MINT_COMPILER_MSVC

#define NOMINMAX // suppress windows.h min/max
#include <intrin.h>

MINT_C_INLINE uint32_t qw_mint_exchange_32_relaxed(mint_atomic32_t *object, uint32_t operand)
{
    return _InterlockedExchange((volatile long*)object, operand);
}

MINT_C_INLINE uint64_t qw_mint_exchange_64_relaxed(mint_atomic64_t *object, uint64_t operand)
{
    return InterlockedExchange64((volatile LONG64*)object, operand);
}

#elif MINT_COMPILER_GCC && (MINT_CPU_X86 || MINT_CPU_X64)

MINT_C_INLINE uint32_t qw_mint_exchange_32_relaxed(mint_atomic32_t *object, uint32_t operand)
{
    // __sync_lock_test_and_set is equivalent to LOCK XCHG on Intel
    // "This built-in function, as described by Intel, is not a traditional test-and-set operation,"
    // "but rather an atomic exchange operation. It writes value into *ptr, and returns the"
    // "previous contents of *ptr."
    // http://gcc.gnu.org/onlinedocs/gcc/_005f_005fsync-Builtins.html#_005f_005fsync-Builtins
    // see also http://stackoverflow.com/questions/8268243/porting-interlockedexchange-using-gcc-intrinsics-only

    return __sync_lock_test_and_set(&(object->_nonatomic), operand);
}

MINT_C_INLINE uint64_t qw_mint_exchange_64_relaxed(mint_atomic64_t *object, uint64_t operand)
{
    return __sync_lock_test_and_set(&(object->_nonatomic), operand);
}

#else

#error /* no support */

#endif


//--------------------------------------------------------------
//  Pointer-sized atomic RMW operation wrappers
//--------------------------------------------------------------
#if MINT_PTR_SIZE == 4

MINT_C_INLINE void* qw_mint_exchange_ptr_relaxed(mint_atomicPtr_t *object, void *operand)
{
    return (void*) qw_mint_exchange_32_relaxed((mint_atomic32_t *) object, (uint32_t)operand);
}

#elif MINT_PTR_SIZE == 8

MINT_C_INLINE void* qw_mint_exchange_ptr_relaxed(mint_atomicPtr_t *object, void *operand)
{
    return (void*) qw_mint_exchange_64_relaxed((mint_atomic64_t *) object, (uint64_t)operand);
}

#else
#error MINT_PTR_SIZE not set!
#endif

#endif /* INCLUDED_QW_ATOMIC_H */

/* -----------------------------------------------------------------------
Last reviewed: May 5, 2014
Last reviewed by: Ross B.
Status: OK
Comments:
- no ARM or PPC support for atomic exchange
-------------------------------------------------------------------------- */
