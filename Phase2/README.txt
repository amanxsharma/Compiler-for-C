Process C function definitions and C expression statements.

Part 1: You must be able to handle function definitions. This includes determining the function return type (you are only responsible for handling functions that return int, char, float, and double), installing the function in the symbol table, generating the necessary assembly code to enter and exit the function, and generating the assembly code for the function body (the statements inside the function). At this level you do not need to handle parameter or local variable declarations.

The only legal statement inside a function is an `expression_statement'. Expression statements can only be made up of: the assignment operator (=), the basic arithmetic binary operators (+,-,*,/), the unary minus arithmetic operator (-), unsigned integer constants, unsigned floating-point constants, the basic comparison operators (==,<,>,!=,<=,>=), global variables (only types int, char, float, or double), parentheses, and function calls to functions that have no parameters. Please note: the result type of an arithmetic operator (+,-,*,/) is the same as the common type of its (binary converted) operands, but the result type of a comparison operator (==,<,>,!=,<=,>=) is always int, regardless of the type of the operands. The type of an assignment operator is the same as the type of its left-hand side, and its value is the same as the value assigned to its left-hand side (that is, after the right-hand side is assignment-converted; see the discussion of conversions below).

Also perform "constant folding" as the expression trees are constructed. Any operator that is detected as having constant operands should be evaluated at compile time (the subtree rooted at the operator is replaced by a constant tree node containing the value of the result).

You are responsible for doing the appropriate C type checking of these expression statements and determining the necessary type conversions. Be sure you understand the C type conversion rules. Implement the "traditional" C rules for type conversions. See the section "C Conversions" below.

For compatibility with legacy code, C (but not C99) allows a function call involving an undeclared function name (it is presumably in another file, or appears later in the current file), and just assumes the function will return a value of type int. To simplify matters, we will adopt the C99 convention and not allow this: every function name appearing in a function call must already be declared or defined (in the same compilation unit). However, you should assume that a function definition without a return type specification declares a function that returns int.

You are also responsible for doing the appropriate semantic error checking. For example, the use of an undeclared identifier in an expression is illegal, even if it is the function name in a function call.

You need not issue an error message for an expression statement that has no side effects. That is, this statement may be processed without a message:

                       x+6;

Above, the value of the expression "x+6" is simply ignored. Since we are not yet implementing the return statement, you do not need to check that the proper type is being returned by a function being defined.

Part 2: In addition to obtaining the 80% level, you must add parameter declarations, parameter references in expressions, and function calls that include parameter lists, including reference parameters. You are not responsible for `old style' definitions of parameters, for example,

int f(a, b, c)
char *a;
int c;
double b;
{ ... }

You are not responsible for detecting parameter number or parameter type mismatches between the definition of a function and a call to that function. You only need to handle parameters of type int, char, float, double, and pointer types (also allow string literals to be passed as parameters). Be sure you understand what it means for a parameter to be of type char or float (Harbison and Steele, Section 9.4).

You will not be calling any functions that have reference parameters (so all arguments to a function call are r-values), but you need to support reference parameters in functions that you define.

Part 3: You must fully implement compound statements and you must add the increment (++) and decrement(--) operators to your repertoire of arithmetic operators. Implementing compound statements means you must support local variable declarations and local variable references within expression statements. Since compound statements can be nested, this means you must be able to implement C's block structured scope rules. You only need to handle local variable declarations of type int, char, float, and double. Note that in the version of C we are implementing, all variable declarations in a block must come before any statements in the block, that is, statements and declarations cannot be mixed.

At all levels you are responsible for detecting the relevant duplicate declarations.

The appropriate back end routines to generate x86 assembly code for this assignment are described in backend-x86.h.

You may assume there will not be any initializers in the source code. You are also not responsible for processing storage class specifiers on any declaration.


C Conversions

There are four kinds of conversions in C: these are called the usual conversions (Harbinson & Steele, C: A Reference Manual (5th ed.), Section 6.3):

Unary conversions. Applied to operands of an operation. The purpose is to reduce the large number of arithmetic types to a smaller number that can be handled by the operators.
Binary conversions. Applied to an operand of a binary operator, based on the type of the other operand. The purpose is to uniformize the types of operands of the binary operator.
Assignment conversions. Applied to the right-hand side of an assignment expression to convert to the type of the left-hand side.
Function argument conversions. Conversions applied to actual arguments in a function call. As with unary conversions, the purpose is to reduce the number of possible types used in the argument list of the function call.
Casting conversions. These are conversions made by an explicit typecast. We will not deal with these kinds of conversions.
We will use the conversion rules for Traditional C rather than Standard C. The rules for the various usual conversions relevant to the project are as follows (here, T represents any data type):

Unary conversions. "float" to "double", "array of T" to "pointer to T", "function returning T" to "pointer to function returning T" , "char" to "int".
Binary conversions. "int" to "double" if the other operand is of type "double".
Assignment conversions. Converts the type of the right-hand side into the type of the left-hand side. Any arithmetic type can be converted to any arithmetic type (this includes "char", "int", "float", and "double"). The constant 0 can be converted to any pointer type.
Function argument conversions. By default (when there is no function prototype), these are the same as the unary conversions (for Traditional C).
Always use the default function argument conversions, even if the function has a prototype. You are not required to check the number and types of actual arguments against the number and types of the corresponding formal parameters (again, even if the function has a prototype).

When a unary operator is applied in an expression and that operator expects an r-value, the r-value of its operand is unary-converted before the operator is applied. When a binary operator (expecting two r-values) is applied, each operand is first unary converted, then the operands are then binary converted (which affects only one of the two operands at most).

The relevant back end routine for applying a conversion operator is b_convert().