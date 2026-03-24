# Project : LLVM front-end for a subset of the C language

Create a front-end for LLVM that
* understands a small subset of the C language
* receives the name of the C file as a command line argument
* generates LLVM IR code for the x86_64 architecture / Linux ABI using the IRBuilder library in LLVM

The C language support is limited to simple C functions like:

```c
int pi100() {
   return 314;
}

int f(int a, int b, int c) {
  return b * b - 4 * a * c;
}

int f(int a, int b, int c) {
  int x, y;
  x = b * b;
  y = 4 * a * c;
  x = x - y;
  return x;
}
```
## Step 1 - functions that evaluate an integer expression
Limitations:
- functions do not have any parameters;
- the return value is int;
- only one statement in function (return)
- only math expressions: + , - , * and parenthees
- all operands in the expression are int constants

## Step 2 - functions that evaluate an expression, including parameters
Limitations: as above, plus
- functions can have zero or more parameters
- all parameters are int;
- no local variables - just parameters, int constants and parentheses

## Step 3 - functions with local variables
Limitations: as above, plus
- all local variables are int
- all local variables are declared on one line
- variables are not initialized when they are declared
- incoming arguments cannot be assigned new values; only local variables can be on the left side of an assigment (lvalues)
- the last instruction in the function is return
- all other instructions are assignments

## Step 4 - functions that call other functions
Limitations: as above plus
- expressions can contain function calls
- calls can be made only to functions previously defined in the same file

## Prerequisites

The assigment requires LLVM development libraries, version 10 or newer .

Recent Linux distributions should already contain one of these LLVM versions; for example in Ubuntu, LLVM can be installed by:
```
sudo apt install llvm llvm-dev
```

To check the version of LLVM use the command: `llvm-config --version`

## Resources
- [Examples](https://gitlab.cs.pub.ro/etti/dcae-public/arh/teaching/aces/chwd/examples) repository: `C3-recursive-parser` - Simple lexer and recursive parser, written in C++, which implements a calculator. Shows how to recognize tokens using the C++ <regex> API and how to use recursive functions to parse an expresion. The repository also contains regex_test - an utility function that can be used to test if a regular expression works on a specific example.
- Examples repository: `C4-llvm-front-end` - LLVM code that generates a simple function. Shows how to use the IRBuilder API in LLVM to generate modules, functions, basic blocks and instructions.
- Examples repository: `C6-llvm-expression-calculator` - Lexer and recursive parser that contains LLVM code. Generates a simple function that implements a mathematical expression.
- [LLVM Programmer’s Manual](https://llvm.org/docs/ProgrammersManual.html)
- [LLVM Intermediate Language Reference Manual](https://llvm.org/docs/LangRef.html)

## Examples of C functions and generated code

The `test` directory contains test C files

Correct tests:
- `testbasic.c` - Step 1: Simple C functions computing arithmetic expressions
- `testfunc.c` - Step 2: Simple to more complex C functions
- `testlocals.c` - Step 3: C functions with local variables
- `testcall.c` - Step 4: C functions calling other functions

Error tests: C files that start with "err_" and contain erroneous C code. When encountering it, the compiler must issue an error message.


The `reference-output` directory contains the output of a correct reference implementation for the following scenarios:
- `test1.output` - Expected output from running step 1.
- `test2.output` - Expected output from running step 2.
- `test3.output` - Expected output from running step 3.
- `test4.output` - Expected output from running step 4.


## How to build and run the program

To build: `make`

To build and run only step 1: `make test1`

To build and run steps 1 and 2: `make test2`

To build and run steps 1 to 3: `make test3`

To build and run all steps: `make test4`

To clean up: `make clean`

Use `make -n` to list the commands required to build and run the code

## How to submit an assignment

* Create a branch using your name (e.g. john.doe) : ```git checkout -b john.doe```
* Complete the "TODO" in the source code
* Push the code on the new branch : ```git push -u origin john.doe```

