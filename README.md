## tpp
tpp (tiny plus-plus) is a tiny compiler designed to compile C++ correctly, as fast as possible. It is inspired by projects like [tcc](https://bellard.org/tcc), [aburi](https://www.github.com/serjective/aburi), etc.

## Goals and Non-Goals
tpp is a compiler written for fun and for learning about compilers.

tpp is actively aiming for C++11 as a baseline.

tpp is not—and will not be—a production-grade optimizing compiler, and by extension, a GCC/Clang replacement.

## Features
NOTICE: tpp is in a very early stage of development, and as such, is not capable of compiling real programs yet! There are also probably going to be bugs, so take that into consideration.

## Usage
Basic usage of tpp is simply invoking `tpp <options> <source_files>`. The compiler supports some flags that will be familiar to you if you've used GCC or Clang before, but here is a quick rundown of some useful ones:
- `-dump-tokens` - This is the equivalent to `-fsyntax-only -Xclang -dump-tokens` in Clang. It is one of the most useful resources for comparing how my lexer fares compared to Clang's.

## Building
tpp itself does not have any external dependencies outside of a C compiler (+ libc) and Make. To build it, clone the repository and run `make`.

This will output the file `build/tpp`.

For Nix users on systems with flakes enabled, there is a devShell present in the `flake.nix` file that contains `gcc`, `clang`, and `tinycc`, as I think it is important to test 

## Supported systems
These are systems that the compiler can build and be executed on, and can be cross-compiled to.
- x86_64-linux
  - This will always have the best support since it is the system I develop the compiler on.

## Contributions
I am willing to accept contributions to the compiler. I don't feel like writing an AI policy so I simply ask that if you use AI like a tool (aka, you actually read the code and use your brain!) then I am perfectly willing to accept your PR's. Yes, I do use some AI on this project, mainly to help catch tokenization/parser bugs (as it is quite annoying for a single person to read a specification over 1000 pages..) and to rewrite some of my code to be more performant/make it more readable as sometimes I tend to write terribly ugly code. My reason for clarifying is I see no difference in explicitly stating I use it just like I state what dependencies are needed for this project. This also makes it easier for people to add this project to their "LLM-ware"/"slopware"/etc. lists if they so choose. This README is also hand written, as it isn't fun to read things people don't want to write themselves. Feel free to use the code in this project for anything, as it is licensed under 0BSD for a reason (and because of my strong distaste for the (A,L)GPL also), and all code not produced by me or an LLM is from various projects I found on Github that are in the public domain.

Also feel free to open issues if you don't want to write code but still want to contribute. Catching lexer/parser, codegen, C++ conformance, and other bugs is just as helpful!