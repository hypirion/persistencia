# Persistent Vector

A persistent vector implementation in C. Not optimised. In particular, you could
theoretically loop unroll it and gain some speed boosts, although this is pure
speculation. Measuring the performance differences would be interesting
though!

The implementations are contained inside the files `pvec_xxx.c`, where `xxx` is
the persistent vector along with its optimisations. `vanilla` is a persistent
vector without any optimisations, `tail` is a tail optimisation, and
`transients` is a transient implementation.

To compile, have your favourite C compiler installed and Boehm-GC available on
your system. The command for compiling should just be

```bash
gcc pvec_xxx.c -lgc
```

If you prefer `clang` (like me), just replace `gcc` with `clang`. Same applies
to other C compilers.

The result is an executable with the original name `a.out`. All implementations
have example usage on how to use the data structure and dot printing functions.
They lie inside the `main` function, at the bottom of each file.

To convert an output dot file named `xxx.dot` to a PNG image, ensure you have
Graphviz installed and call the command

```bash
dot -Tpng -o xxx.png xxx.dot
```

If you want to convert all the files in this directory, then this command should
work on most (all?) unix shells:

```bash
for f in foo*.dot; do dot -Tpng -o $(basename $f .dot).png $f; done
```
