## tpp
tpp (tiny plus-plus) is a tiny compiler designed to compile C++ correctly, as fast as possible. It is inspired by projects like [tcc](https://bellard.org/tcc), [aburi](https://www.github.com/serjective/aburi), etc.

## Goals and Non-Goals
tpp **is** a compiler written for fun and for learning about compilers.

tpp **is** aiming for C++11 as a baseline.

tpp **is not** a production-grade optimizing compiler, and by extension, a GCC/Clang replacement.

tpp **is not** going to work on every system. At the moment I test on a x86_64 NixOS box and a Hackintosh running macOS Sonoma.

## Building
tpp itself does not have any external dependencies outside of a libc and Make. To build it, clone the repository and run
```
make
```
This will output the file `build/tpp`.

## Supported systems
I'll add a chart here later, but currently tpp is so early into development it does not compile stuff yet