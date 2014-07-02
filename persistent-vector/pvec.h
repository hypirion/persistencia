/*
 * Copyright (c) 2014 Jean Niklas L'orange. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef PVEC_H
#define PVEC_H

#include <stdint.h>

#ifndef PVEC_BITS
// PVEC_BITS is b, the total amount of bits used per node in the trie.
#define PVEC_BITS 2

// PVEC_MAX_HEIGHT is the maximal height of a trie. With 32 bits and b = 5, we
// can at most have 2**32 - 1 elements, which is equal to 7 levels. However, for
// illustration purposes, we use b = 2, meaning we can have at most 16 levels.
#define PVEC_MAX_HEIGHT 16
#endif

// PVEC_BRANCHING is the branching factor. With n bits, we have 2**n elements in
// each trie node.
#define PVEC_BRANCHING (1 << PVEC_BITS)

// PVEC_MASK is the mask used to efficiently perform modulo for bitwise vectors.
#define PVEC_MASK (PVEC_BRANCHING - 1)

// An opaque persistent vector struct.
typedef struct _Pvec Pvec;

// pvec_create returns an empty persistent vector.
const Pvec* pvec_create(void);

// pvec_count returns the size of this persistent vector.
uint32_t pvec_count(const Pvec *pvec);

// pvec_nth returns the element stored at the given index.
void* pvec_nth(const Pvec *pvec, uint32_t index);

// pvec_pop returns a new persistent vector with the last element removed.
const Pvec* pvec_pop(const Pvec *pvec);

// pvec_peek returns the last element in this persistent vector.
void* pvec_peek(const Pvec *pvec);

// pvec_push returns a new persistent vector with the last element appended onto
// this persistent vector.
const Pvec* pvec_push(const Pvec *restrict pvec, const void *restrict elt);

// pvec_update returns a new persistent vector where the element at the given
// index is replaced with the new element.
const Pvec* pvec_update(const Pvec *restrict pvec, uint32_t index, const void *restrict elt);

// pvec_right_slice returns a new persistent vector with the new given size. The
// new size should be less than or equal the current size.
const Pvec* pvec_right_slice(const Pvec *pvec, uint32_t new_size);

// const Pvec* pvec_concat(const Pvec *left, const Pvec *right);
// const Pvec* pvec_slice(const Pvec *pvec, uint32_t from, uint32_t to);

#ifdef TRANSIENT_PVEC

typedef struct _TransientPvec TransientPvec;

TransientPvec* pvec_to_transient(const Pvec *pvec);
const Pvec* transient_to_pvec(TransientPvec *tpvec);

uint32_t transient_pvec_count(const TransientPvec *tpvec);
void* transient_pvec_nth(const TransientPvec *tpvec, uint32_t index);
TransientPvec* transient_pvec_pop(TransientPvec *tpvec);
void* transient_pvec_peek(const TransientPvec *tpvec);
TransientPvec* transient_pvec_push(TransientPvec *restrict tpvec, const void *restrict elt);
TransientPvec* transient_pvec_update(TransientPvec *restrict tpvec, uint32_t index, const void *restrict elt);
#endif
#endif
