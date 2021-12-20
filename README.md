# Raft

This project originally started as an attempt to build a basic Forth interpreter in C as a programming exercise. However, it very quickly began evolving into its own distinct language. As it is in the very early stages of active development, many features may be broken or partially-implemented. As such, it is not advisable to rely on functionality until things have become more stable.
The nature of this project is inspired by Alexey Kutepov (Tsoding)'s [Porth](https://gitlab.com/tsoding/porth) language.

## Compiling the Interpreter

The build script for Windows, ```build-win64.cmd``` is set up for use with a MINGW-w64 GCC installation. As the compilation does not require any special flags, options, or features, you can build it with any C compiler.

A build script for Unix-like systems will be added soon.

## Project Milestones

- [ ] Language Features
    - [ ] Types
        - [ ] Integers
        - [ ] Strings (Stored as ASCII values, followed by a length integer)
    - [ ] Arithmetic
        - [ ] Addition
        - [ ] Subtraction
        - [ ] Multiplication
        - [ ] Euclidean Division
    - [ ] Boolean
        - [ ] Greater-than
        - [ ] Less-than
        - [ ] Equivalent-to
        - [ ] And
        - [ ] Or
    - [ ] Stack Manipulation
        - [ ] Swap
        - [ ] Rotate
        - [ ] Over
        - [ ] Duplicate
        - [ ] Drop
    - [ ] Screen Output
        - [ ] Integers
        - [ ] Strings
        - [ ] General-use spaces
        - [ ] Line-breaks
    - [ ] Loops
        - [ ] Do-While
        - [ ] For
        - [ ] While
    - [ ] Conditionals
        - [ ] If-Then-Else
    - [ ] Structural Navigation
        - [ ] Goto
        - [ ] Return/Exit
- [ ] Error-checking
    - [x] Unknown keyword detection
    - [ ] Typechecking (in progress)


## Commands & Operations

Check out doc/reference.txt for list of implemented operations.
