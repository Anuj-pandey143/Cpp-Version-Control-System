#!/bin/bash

# A script to compile the Time-Travelling File System project.
# This makes the compilation process easy and repeatable.

echo "Compiling source files..."

# g++ is the compiler command.
# -std=c++17 tells the compiler to use the C++17 standard.
# -o anuj names the final executable file 'anuj'.
# We now only need to compile the single MainCode.cpp file.
g++ -std=c++17 MainCode.cpp -o anuj

echo "Compilation complete."
echo "To run the program, use the command: ./anuj"