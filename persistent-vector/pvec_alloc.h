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

#ifndef PVEC_ALLOC_H
#define PVEC_ALLOC_H

/*
 * This is just an allocation header which depends on Boehm GC. It's done to
 * make the code easy to understand: Including refcounting makes the code very
 * hard to read.
 */

#include <gc/gc.h>

// Allocation of memory which may contain pointers. The returned contents must
// be nulled (like calloc).
#define PVEC_MALLOC GC_MALLOC

// Reallocation of memory, which may contain pointers. If the allocation is
// widened, the extra memory must be nulled (like calloc).
#define PVEC_REALLOC GC_REALLOC

// Allocation of memory which will never contain pointers. The returned contents
// must be nulled (like calloc).
#define PVEC_MALLOC_ATOMIC GC_MALLOC_ATOMIC

#endif
