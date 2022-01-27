# Raft

This project originally started as an attempt to build a basic Forth interpreter in C as a programming exercise. However, it very quickly began evolving into its own distinct language. As it is in the very early stages of active development, many features may be broken or partially-implemented. As such, it is not advisable to rely on functionality until things have become more stable.

Parts of this project take inspiration from Alexey Kutepov (Tsoding)'s [Porth](https://gitlab.com/tsoding/porth) language.

## Compiling the Interpreter

At this time, there are build-scripts for both Linux and Windows. The Linux script should be relatively universal and shouldn't need modification to run on most systems. 
On the other hand, the Windows build-script may require modification. By default, it is configured for GCC on Windows using MINGW-w64. Your specific path/version/compiler may vary,
so double check the script if it doesn't seem to be compiling.

The compiled executable (`raft` or `raft.exe`) will be placed in the `./build/` directory.

## Project Milestones

- [ ] Language Features
    - [x] Types
        - [x] Integers
        - [x] Strings (Stored as ASCII values, followed by a length integer)
    - [x] Arithmetic
        - [x] Addition
        - [x] Subtraction
        - [x] Multiplication
        - [x] Euclidean Division
    - [x] Boolean
        - [x] Greater-than
        - [x] Less-than
        - [x] Equivalent-to
        - [x] And
        - [x] Or
    - [x] Stack Manipulation
        - [x] Swap
        - [x] Rotate
        - [x] Over
        - [x] Duplicate
        - [x] Drop
    - [x] Screen Output
        - [x] Integers
        - [x] Strings
        - [x] General-use spaces
        - [x] Line-breaks
    - [ ] Loops
        - [x] Do-While
        - [ ] For
        - [ ] While
    - [ ] Conditionals
        - [x] If
        - [ ] Then-Else branching (requires if)
    - [ ] Modularity
        - [ ] Functions
        - [ ] Libraries (requires functions)
    - [ ] Structural Navigation
        - [x] Goto
        - [ ] Return/Exit
    - [ ] Variables & Constants
        - [ ] Integer constants
        - [ ] Integer variables
        - [ ] String constants
        - [ ] String variables
    - [ ] Memory Manipulation
        - [ ] Pointers
    - [ ] I/O
        - [ ] User input
        - [ ] Read from files
        - [ ] Write to files
- [ ] Interpreter Features
    - [x] Help menu
    - [x] Flags
    - [x] Read from file
    - [ ] Interactive mode (Python-style interpreter)
    - [x] Pre-processing stage (required for typechecking/forward jumps) 
    - [ ] Error-checking
        - [x] Unknown keyword detection
        - [ ] Typechecking (in progress)
    - [ ] Forth-compatible mode

More may be added in the future as I come up with features I would like to implement. Feature suggestions may be submitted to this project as issues. When submitting a feature request, please include the tag [FEATURE] at the beginning of the subject line.

## Commands & Operations

Check out doc/reference.txt for list of implemented operations. Further documentation coming soon.
