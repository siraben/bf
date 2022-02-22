# Brainfuck interpreter in C

![Mandelbrot set program](./mandel.png)

This is an interpreter for Brainfuck I initially wrote in an evening
in C.  The interpreter is essentially as fast as it can be without
resorting to JIT compilation, because the following optimizations were
performed:

- precomputing jump lengths between `[` and `]`
- precomputing runs of `+`, `-`, `>`, `<`

In terms of relative speedups, here's what my MacBookPro18,3 reports
(optimizations are cumulative, so subsequent ones include previous
ones).

| Optimization     | Time to render mandelbrot set (s) |
|------------------|-----------------------------------|
| none             | 28.57                             |
| precompute jumps | 15.25                             |
| precompute runs  | 7.71                              |
| no flush on .    | 6.49                              |

See the Mandelbrot set with

```ShellSession
$ nix run github:siraben/bf <(curl -s https://raw.githubusercontent.com/erikdubbelboer/brainfuck-jit/master/mandelbrot.bf)
```

## Building
With Nix, run `nix build github:siraben/bf`.

```ShellSession
$ gcc -O2 -o bf bf.c
```

## Tested Programs
- Mandelbrot set
- Hello world
- ROT13
- [Lost Kingdom](https://github.com/rdebath/LostKingdom) (text
  adventure game)
