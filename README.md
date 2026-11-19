## tpp
tpp (tiny plus-plus) is a tiny compiler designed to compile C++ correctly, as fast as possible. It is inspired by projects like [tcc](https://bellard.org/tcc), [aburi](https://www.github.com/serjective/aburi), etc.

## Goals and Non-Goals
tpp is written for fun and to help me learn about compilers.

tpp is aiming for C++11 as a baseline.

tpp is not—and will not be—a production-grade optimizing compiler, and by extension, a GCC/Clang replacement.

## Features
NOTICE: tpp is in a very early stage of development, and as such, is not capable of compiling real programs yet! There are also probably going to be bugs, so take that into consideration.

## Usage
Basic usage of tpp is simply invoking `tpp <options> <source_files>`. The compiler supports some flags that will be familiar to you if you've used GCC or Clang before, but here is a quick rundown of some useful ones:
- `-dump-tokens` - This is the equivalent to `-fsyntax-only -Xclang -dump-tokens` in Clang. It is one of the most useful resources for comparing how my lexer fares compared to Clang's.

## Building
tpp itself does not have any external dependencies outside of a C compiler, libc, and Make. To build it, clone the repository and run `make`. This will output a file at `build/tpp`.

For Nix users on a system with flakes enabled, you can run `nix develop` to enter a devshell with the tools you need to build tpp.

## Contributing
Pull requests are welcome! I'm fine accepting PR's that use AI but you must understand *all* of the code and you are required to hand-write the description since I don't want to read a wall of slop you were too lazy to type yourself. If you don't want to contribute, opening issues is also greatly appreciated.