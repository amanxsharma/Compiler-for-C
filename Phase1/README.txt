This directory contains the files comprising the skeleton of the C
compiler "pcc3".


Source files:

Makefile	- Compiler maintenance
defs.h		- Global definitions
gram.y		- The skeleton grammar for Pascal
main.c		- The main routine
message.c	- Message handling routines
message.h	- Definitions for message.c
scan.l		- The specification of the lexical scanner
symtab.c	- Symbol table maintenance
symtab.h	- Definitions for symtab.c
types.c		- Processes Pascal type information
types.h		- Definitions for types.c
encode.h    - Definitions for encode.c
encode.c    - Code for spitting out assembly code
tree.h      - Definitions for tree.c
tree.c      - Main code
backend-x86.c	- Routines for generating x86 assembly code
backend-x86.h	- Definitions for backend-x86.c
utils.c		- Miscellaneous utilities

Documentation:

README.txt	- This file

Description:
Process C global variable declarations. This involves both installing the declarations into the symbol table and allocating memory for the variables in the assembly language output file. Also, after all declarations have been processed, you should dump the symbol table (using st_dump() from symtab.h); to do this, run your executable with the "-d" or "--dump" option as a command line argument.

Your compiler should read C source code from stdin and write the x86 assembly language output to stdout. Your compiler executable should be called pcc3. You will not have to emit assembly code explicitly, but rather call appropriate routines in the back end (backend-x86.c and backend-x86.h). Besides altering the gram.y file, put syntax tree-building functions into a new file tree.c, with definitions for export in tree.h. Put code-generating routines into a new file encode.c, with definitions for export in encode.h. With few exceptions throughout the project, all backend routines are called from encode.c (some may be called directly from the grammar). No backend routines should be called from tree.c, hence you will not need to include backend-x86.h in tree.c.


Part 1: You must be able to process the following basic type specifiers: int, char, float, and double. You may limit the syntax so that only one type specifier may be given per declaration. You must also be able to handle pointer and array type modifiers. You may limit the syntax so that array dimensions must always be given. You may assume the dimension given will always be an unsigned integer constant. Each declaration should include an identifier (id). If not, an error should be issued. A symbol table entry should be made for each id. The entry should indicate the type of the declaration. Routines for building and analyzing types are in the types module (types.h) and bucket module (bucket.h), and routines for manipulating the symbol table are in the symbol table module (symtab.h). You are required to use these modules, but you are not allowed to modify them. For more on these and the other modules, see the Resources section, below.

Part 2: You should also allow multiple type specifiers per declaration. You should handle the additional specifiers signed, unsigned, short, and long. You should add the necessary semantic checks and error messages to support multiple type specifiers (e.g., short short, unsigned double, et cetera are illegal). You should also add the function type modifier. You should add the necessary semantic checks and error messages to support function modifiers (it is illegal for a function to return a function, for example). Only "old style" functions need to be supported at this level, that is, with no parameter list between parentheses.

Part 3: You should also allow parameters in function declarations. You should insist that each parameter declaration includes an id (else semantic error). The possible parameter types are the same as described in the previous levels, including pointers, arrays, and functions. You should also support the void return type for a function. A parameter may be a reference parameter, e.g.,

    int f(int& a);
    void g(int (&a)[5]);

This is the only aspect of the language that is not part of C. You can assume that any "&" appears only once in a parameter declaration, and only modifies the complete parameter type (so for example, you will never see int h(int&* a);). You can also assume that any parameter of function type has no parameter declarations of its own (you will only see "old style" function types as parameters).

The semantic errors you should check for at this level are that each parameter declaration must include an id, and that the same id should not appear more than once in the same parameter list.
