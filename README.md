# jpl-compiler
Personal compiler project for a fictional language from the university
Language is JPL, a semi-functional language that uses multi-dimensional array comprehension for loops.
Compiler is developed in C on linux.
When in the main folder, use "make" to compile, or "make run TEST=[filename]" to compile and run the compiler.
hw1/examples contains some sample .jpl programs to test the compiler.

## 1/18/2025
Compiler's lexer is finished. When run in lex mode (using the "-l" flag) the lexer will print to STDOUT an ordered list of all tokens that appear in the .jpl file.
