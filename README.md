# RA_c

Simple Relational Algebra implementation in C++20 as an exercise.

This version is designed around storage that can be accessed via dynamic types,
to allow for easily interfacing with other languages.

A separate exercise will cover a C++ only, fully static implementation.

Only tested on Linux x86_64.

Compile with:

    gcc-10 -std=c++20 -Wpedantic -ggdb -O3 -I ${EXT}/Catch2/single_include/catch2/  -I . test.cpp -o test -lstdc++ -lm
