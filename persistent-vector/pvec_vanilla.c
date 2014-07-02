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

/*
 * This is the vanilla implementation of a persistent vector. This does not
 * include a tail, transient conversions, nor a display.
 */

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "pvec.h"
#include "pvec_alloc.h"

// This is a trie node. It is always the branching factor size (unused table
// entries will be NULL).
typedef struct Node {
  struct Node *child[PVEC_BRANCHING];
} Node;

struct _Pvec {
  // The size of the vector.
  uint32_t size;
  // The height of the vector, represented as a shift.
  uint32_t shift;
  // The root of the vector.
  Node* root;
};

static Node EMPTY_NODE = {.child = {0}};

// An empty vector. (Not necessarily the only empty vector!)
static Pvec EMPTY_VECTOR = {.size = 0, .shift = 0, .root = &EMPTY_NODE};

// These are just prototypes -- no need to worry about these.
static inline Node *node_create(void);
static inline Node *node_clone(const Node* node);
static inline Pvec* pvec_clone(const Pvec *pvec);

// pvec_create just returns the empty vector.
const Pvec* pvec_create() {
  return &EMPTY_VECTOR;
}

// pvec_count just returns the size value inside the vector head.
uint32_t pvec_count(const Pvec *pvec) {
  return pvec->size;
}

void* pvec_nth(const Pvec *pvec, uint32_t index) {
  Node *node = pvec->root;
  for (uint32_t s = pvec->shift; s > 0; s -= PVEC_BITS) {
    uint32_t subindex = (index >> s) & PVEC_MASK;
    node = node->child[subindex];
  }
  // This last call is here because unsigned integers cannot be negative, thus
  // `s >= 0` will always be true.
  return (void *) node->child[index & PVEC_MASK];
}

void* pvec_peek(const Pvec *pvec) {
  return pvec_nth(pvec, pvec_count(pvec) - 1);
}

const Pvec* pvec_update(const Pvec *restrict pvec, uint32_t index,
                        const void *restrict elt) {
  Pvec *clone = pvec_clone(pvec);
  Node *node = node_clone(pvec->root);
  clone->root = node;
  for (uint32_t s = pvec->shift; s > 0; s -= PVEC_BITS) {
    uint32_t subindex = (index >> s) & PVEC_MASK;
    node->child[subindex] = node_clone(node->child[subindex]);
    node = node->child[subindex];
  }
  uint32_t subindex = index & PVEC_MASK;
  node->child[subindex] = (Node *) elt;
  return (const Pvec*) clone;
}

// pvec_push is equivalent to the append function described in Section 2.5, but
// with bitwise access tricks.
const Pvec* pvec_push(const Pvec *restrict pvec, const void *restrict elt) {
  Pvec *clone = pvec_clone(pvec);
  uint32_t index = pvec_count(pvec);
  clone->size = pvec->size + 1;
  // this is the d_full(P) check for bit vectors
  if (pvec_count(pvec) == (PVEC_BRANCHING << pvec->shift)) {
    Node *new_root = node_create();
    new_root->child[0] = pvec->root;
    clone->root = new_root;
    clone->shift = pvec->shift + PVEC_BITS;
  }
  else {
    clone->root = node_clone(pvec->root);
  }
  Node *node = clone->root;
  for (uint32_t s = clone->shift; s > 0; s -= PVEC_BITS) {
    uint32_t subindex = (index >> s) & PVEC_MASK;
    if (node->child[subindex] == NULL) { // the create part of clone-or-create
      node->child[subindex] = node_create();
    }
    else { // the clone part of clone-or-create
      node->child[subindex] = node_clone(node->child[subindex]);
    }
    node = node->child[subindex];
  }
  uint32_t subindex = index & PVEC_MASK;
  node->child[subindex] = (Node *) elt;
  return (const Pvec*) clone;
}

const Pvec* pvec_pop(const Pvec *pvec) {
  Pvec *clone = pvec_clone(pvec);
  uint32_t index = pvec_count(pvec) - 1;
  clone->size = pvec_count(pvec) - 1;
  if (pvec_count(clone) == (1 << pvec->shift) && pvec->shift > 0) {
    clone->shift = pvec->shift - PVEC_BITS;
    clone->root = pvec->root->child[0];
    return clone;
  }
  else {
    Node *node = node_clone(pvec->root);
    clone->root = node;
    for (uint32_t s = pvec->shift; s > 0; s -= PVEC_BITS) {
      uint32_t subindex = (index >> s) & PVEC_MASK;
      if ((index & (((2*PVEC_BITS) << s) - 1)) == 0) {
        node->child[subindex] = NULL;
        return clone;
      }
      else {
        node->child[subindex] = node_clone(node->child[subindex]);
        node = node->child[subindex];
      }
    }
    node->child[index & PVEC_MASK] = NULL;
    return clone;
  }
}

// Performing a right slice on a persistent vector. Implemented in Scala (with
// displays), but not in Clojure.
const Pvec* pvec_right_slice(const Pvec *pvec, uint32_t new_size){
  Pvec *clone = pvec_clone(pvec);
  uint32_t index = new_size;
  clone->size = new_size;

  // We have to cut the tree until the height is minimal
  while (pvec_count(clone) <= (1 << clone->shift) && clone->shift > 0) {
    clone->shift = clone->shift - PVEC_BITS;
    clone->root = clone->root->child[0];
  }

  // The tree being fully dense is a special case, and is short-circuited
  if (pvec_count(clone) == (PVEC_BRANCHING << clone->shift)) {
    return clone;
  }
  
  // Notice that this part is almost exactly the same as the `else` part within
  // the pvec_pop function. The only difference is the memset functions to
  // ensure that all elements right of the trie to walk is nilled through
  // the memset function.
  Node *node = node_clone(clone->root);
  clone->root = node;
  for (uint32_t s = clone->shift; s > 0; s -= PVEC_BITS) {
    uint32_t subindex = (index >> s) & PVEC_MASK;
    if ((index & (((2*PVEC_BITS) << s) - 1)) == 0) {
      memset(&node->child[subindex], 0,
             (PVEC_BRANCHING - subindex) * sizeof(Node *));
      return clone;
    }
    else {
      node->child[subindex] = node_clone(node->child[subindex]);
      memset(&node->child[subindex + 1], 0,
             (PVEC_BRANCHING - (subindex - 1)) * sizeof(Node *));
      node = node->child[subindex];
    }
  }
  uint32_t subindex = index & PVEC_MASK;
  memset(&node->child[subindex + 1], 0,
         (PVEC_BRANCHING - (subindex - 1)) * sizeof(Node *));
  return clone;
}

// Inline helper functions

static inline Node *node_create(void) {
  Node *new = PVEC_MALLOC(sizeof(Node));
  return new;
}

static inline Node *node_clone(const Node* node) {
  Node *clone = PVEC_MALLOC(sizeof(Node));
  memcpy(clone, node, sizeof(Node));
  return clone;
}

static inline Pvec* pvec_clone(const Pvec *pvec) {
  Pvec *clone = PVEC_MALLOC(sizeof(Pvec));
  memcpy(clone, pvec, sizeof(Pvec));
  return clone;
}

// Persistent vector dot printing functions. Some are internal, others are
// external. See pvec.h for those who are external.

// Not needed to understand the persistent vector, but may be handy to print
// them out in dot format.

static void pvec_to_dot_rec(FILE *out, Node *root, uint32_t shift, uint32_t size) {
  fprintf(out,
          "  s%p [label=<\n<table border=\"0\" cellborder=\"1\" "
          "cellspacing=\"0\" cellpadding=\"6\" align=\"center\" port=\"body\">\n"
          "  <tr>\n", root);
  if (shift == 0) {
    uint32_t i;
    for (i = 0; i < size; i++) {
      uintptr_t leaf = (uintptr_t) ((void **) root->child)[i];
      fprintf(out,
              "    <td height=\"36\" width=\"25\">%lx</td>\n",
              leaf);
    }
    while (i < PVEC_BRANCHING) {
      fprintf(out, "    <td height=\"36\" width=\"25\"></td>\n");
      i++;
    }
    fprintf(out,
            "  </tr>\n"
            "</table>>];\n");
  } else {
    for (uint32_t i = 0; i < PVEC_BRANCHING; i++) {
      fprintf(out,
              "    <td height=\"36\" width=\"25\" port=\"%u\"></td>\n",
              i);
    }
    fprintf(out,
            "  </tr>\n"
            "</table>>];\n");
    uint32_t child_size = (1 << shift);
    uint32_t i = 0;
    while (size > child_size) {
      size -= child_size;
      fprintf(out, "  s%p:%u -> s%p:body;\n", root, i, root->child[i]);
      pvec_to_dot_rec(out, root->child[i], shift - PVEC_BITS, child_size);
      i++;
    }
    if (size > 0) {
      child_size = size;
      fprintf(out, "  s%p:%u -> s%p:body;\n", root, i, root->child[i]);
      pvec_to_dot_rec(out, root->child[i], shift - PVEC_BITS, child_size);
    }
  }
}

void pvec_to_dot(Pvec *vec, char *loch) {
  FILE *out = fopen(loch, "w");
  fprintf(out, "digraph g {\n  bgcolor=transparent\n  node [shape=none];\n");
  fprintf(out,
          "  s%p [label=<\n<table border=\"0\" cellborder=\"1\" "
          "cellspacing=\"0\" cellpadding=\"6\" align=\"center\" port=\"body\">\n"
          "  <tr>\n"
          "    <td height=\"36\" width=\"25\">%d</td>\n"
          "    <td height=\"36\" width=\"25\">%d</td>\n"
          "    <td height=\"36\" width=\"25\" port=\"root\"></td>\n"
          "  </tr>\n"
          "</table>>];\n",
          vec, pvec_count(vec), vec->shift);
  fprintf(out, "  s%p:root -> s%p:body;\n", vec, vec->root);
  pvec_to_dot_rec(out, vec->root, vec->shift, pvec_count(vec));
  fprintf(out, "}\n");
  fclose(out);
}

typedef struct {
  unsigned int size;
  unsigned int cap;
  void **ptrs;
} ArraySet;

static int arr_member(ArraySet *aref, void *elt) {
  for (unsigned int i = 0; i < aref->size; i++) {
    if (elt == aref->ptrs[i]){
      return 1;
    }
  }
  return 0;
}

static void arr_insert(ArraySet *aref, void *elt) {
  if (aref->size == aref->cap) {
    aref->cap <<= 1;
    aref->ptrs = (void **) PVEC_REALLOC(aref->ptrs, sizeof(void *) * aref->cap);
  }
  aref->ptrs[aref->size] = elt;
  aref->size++;
}

static ArraySet *arr_create() {
  ArraySet *aref = (ArraySet *) PVEC_MALLOC(sizeof(ArraySet));
  aref->size = 0;
  aref->cap = 32; // Arbitrary value
  aref->ptrs = (void **) PVEC_MALLOC(sizeof(void *) * aref->cap);
  return aref;
}

static const int colour_size = 5;
static char *colours[5] =
  {"burlywood3", "cadetblue3", "darkolivegreen3", "gold3", "pink3"};

static void node_to_dot_rec(FILE* out, char *colour, Node *root, uint32_t shift,
                            uint32_t size, ArraySet *aref) {
  if (arr_member(aref, (void *) root)) {
    return;
  }
  else {
    arr_insert(aref, (void *) root);
  }
  fprintf(out,
          "  s%p [color=%s, label=<\n<table border=\"0\" cellborder=\"1\" "
          "cellspacing=\"0\" cellpadding=\"6\" align=\"center\" port=\"body\">\n"
          "  <tr>\n",
          root, colour);
  if (shift == 0) {
    uint32_t i;
    for (i = 0; i < size; i++) {
      uintptr_t leaf = (uintptr_t) ((void **)root->child)[i];
      fprintf(out, "    <td height=\"36\" width=\"25\">%lx</td>\n", leaf);
    }
    while (i < PVEC_BRANCHING) {
      fprintf(out, "    <td height=\"36\" width=\"25\"></td>\n");
      i++;
    }
    fprintf(out,
            "  </tr>\n"
            "</table>>];\n");
  } else {
    for (uint32_t i = 0; i < PVEC_BRANCHING; i++) {
      fprintf(out,
              "    <td height=\"36\" width=\"25\" port=\"%u\"></td>\n",
              i);
    }
    fprintf(out,
            "  </tr>\n"
            "</table>>];\n");
    uint32_t child_size = (1 << shift);
    uint32_t i = 0;
    while (size > child_size) {
      size -= child_size;
      fprintf(out, "  s%p:%u -> s%p:body [color=%s];\n", root, i, root->child[i],
              colour);
      node_to_dot_rec(out, colour, root->child[i], shift - PVEC_BITS,
                      child_size, aref);
      i++;
    }
    if (size > 0) {
      child_size = size;
      fprintf(out, "  s%p:%u -> s%p:body [color=%s];\n", root, i, root->child[i],
              colour);
      node_to_dot_rec(out, colour, root->child[i], shift - PVEC_BITS,
                      child_size, aref);
    }
  }
}

static void pvecs_to_dot_rec(FILE* out, Pvec *vec, char *colour, ArraySet *aref) {
  fprintf(out,
          "  s%p [color=%s, label=<\n<table border=\"0\" cellborder=\"1\" "
          "cellspacing=\"0\" cellpadding=\"6\" align=\"center\" port=\"body\">\n"
          "  <tr>\n"
          "    <td height=\"36\" width=\"25\">%d</td>\n"
          "    <td height=\"36\" width=\"25\">%d</td>\n"
          "    <td height=\"36\" width=\"25\" port=\"root\"></td>\n"
          "  </tr>\n"
          "</table>>];\n",
          vec, colour, pvec_count(vec), vec->shift);
  fprintf(out, "  s%p:root -> s%p:body [color=%s];\n", vec, vec->root, colour);
  node_to_dot_rec(out, colour, vec->root, vec->shift, pvec_count(vec), aref);
}

void pvecs_to_dot(Pvec **vec, int n, char *loch) {
  ArraySet *aref = arr_create();

  FILE *out = fopen(loch, "w");
  fprintf(out, "digraph g {\n  bgcolor=transparent\n  node [shape=none];\n");

  for (int i = 0; i < n; i++) {
    pvecs_to_dot_rec(out, vec[i], colours[i % colour_size], aref);
  }

  fprintf(out, "}\n");
  fclose(out);
}

int main() {
  const Pvec *p = pvec_create();
  for (uintptr_t i = 0; i < 100; i++) {
    p = pvec_push(p, (void *) (i + 1));
    int ok = 1;
    for (uint32_t j = 0; j <= i; j++) {
      uintptr_t n = (uintptr_t) pvec_nth(p, j);
      if (n != j + 1) {
        ok = 0;
      }
    }
    if (!ok) {
      printf("For %lu, not ok\n", i);
    }
  }
  for (uint32_t i = 0; i < 17; i++) {
    const Pvec *q = pvec_right_slice(p, i);
    char str[80];
    sprintf(str, "vanilla-%u.dot", i);
    pvec_to_dot(q, str);
  }
  Pvec *multi[2] = {pvec_right_slice(p, 4), pvec_right_slice(p, 16)};
  pvecs_to_dot(&multi, 2, "vanilla-multi.dot");
}
