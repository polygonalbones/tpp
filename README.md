## tpp
tpp (tiny plus-plus) is a tiny compiler designed to compile C++ correctly, as fast as possible. It is inspired by projects like [tcc](https://bellard.org/tcc), [aburi](https://www.github.com/serjective/aburi), etc.

## Goals and Non-Goals
tpp **is** a compiler written for fun. I do not use AI tools in this project specifically because I want to learn and have fun with it.

tpp **is** aiming for C++11 as a baseline.

tpp **is not** a production-grade optimizing compiler, and by extension, a GCC/Clang replacement.

tpp **is not** guaranteed to work on all systems. At the moment I test on a x86_64 NixOS box and a Hackintosh running macOS Sonoma.

## Building
tpp itself does not have any external dependencies, so you just need to grab [Zig 0.17.1](https://ziglang.org/download/#release-0.17.1) and run `zig build` inside the directory after cloning it.

## Supported systems
I'll add a chart here later, but currently tpp is so early into development it does not compile stuff yet