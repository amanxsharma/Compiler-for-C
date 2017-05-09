/*
 *
 *   backend-x86.h 
 *
 *   definitions and function prototypes for backend-x86.c
 *
 */

#ifndef BACKEND_H
#define BACKEND_H
#include "types.h"


/* Macro and type definitions */

#define NOFF -1

/* Offset from the frame pointer where the reference link for a local
   Pascal function is stored (also the offset of the first parameter) */
#define FUNC_LINK_OFFSET 8

/* Maximum allowable depth of function call nesting */
#define MAX_CALL_NEST  128

/* Sections of the executable program */
typedef enum { SEC_NONE, SEC_TEXT, SEC_RODATA, SEC_DATA } ASM_SECTION;

/* Jump conditions */
typedef enum { B_ZERO, B_NONZERO } B_COND;

/* Arithmetic and comparison operations */
typedef enum { B_ADD, B_SUB, B_MULT, B_DIV, B_MOD,
               B_LT, B_LE, B_GT, B_GE, B_EQ, B_NE } B_ARITH_REL_OP;

/* Increment and decrement operations */
typedef enum { B_PRE_INC, B_POST_INC, B_PRE_DEC, B_POST_DEC } B_INC_DEC_OP;



/**************************
 *                        *
 * Routines for Project 1 *
 *                        *
 **************************/


/* b_global_decl emits the pseudo-op .data if beginning a data
   section.  In any case, it emits the pseudo-op .global for a global variable
   and a label for that variable, as well as an .align to the appropriate
   alignment and a .size to the appropriate size.  A typical simple variable
   declaration (e.g. an int) is accomplished by a call to b_globl_decl followed
   by a call to the appropriate b_alloc function for initialized data
   (b_alloc_int in the case of an int), or to b_skip() for uninitialized data.

   For example, to emit code for the global declaration ``int i=5;'',
   one might call

   b_global_decl("i", sizeof(int), sizeof(int));
   b_alloc_int(5);

   For another example, to emit code for the global declaration
   ``int a[10] = {3,4,5};'', one might call

   b_global_decl("a", sizeof(int), 10*sizeof(int));
   b_alloc_int(3);
   b_alloc_int(4);
   b_alloc_int(5);
   b_skip(7*sizeof(int));
   */
void b_global_decl (char *id, int alignment, unsigned int size);

/* The following seven functions emit code to allocate space for
   characters, short integers, integers, long integers, pointers, floats,
   and doubles, respectively.  In all cases, init is a required 
   initialization for the variable.  b_global_decl should be called
   once beforehand for the variable name (see header comments for this
   function).

   To allocate a pointer (and initialize it to 0), pass "0" (the string
   constant) as the first argument to b_alloc_ptr().
*/
void b_alloc_char (int init);
void b_alloc_short (int init);
void b_alloc_int (int init);
void b_alloc_long (long init);
void b_alloc_ptr (char *init);
void b_alloc_float (double init);
void b_alloc_double (double init);

/* b_skip() allocates a given number of bytes of space by advancing the
   location counter by the number.  These bytes are (presumably) zeroed.
   Use b_skip() after a truncated initialization list.  */

void b_skip(unsigned int amount);


/* This is not really a back end function.  It is defined in the utils.c
   file in the Project 1 solution.  It provides a uniform interface to the
   seven functions above.
*/
void b_alloc_gdata(TYPETAG tag, ...);




/**************************
 *                        *
 * Routines for Project 2 *
 *                        *
 **************************/

/*****                                *****
 ***** Expression evaluation routines *****
 *****                                *****/

/* Pop a datum off the control stack (used in expression evaluation)
*/
#define  b_pop()  b_internal_pop(TRUE)

/* Not for external use; use b_pop() instead.
   Removes and discards a value from the top of the stack.  If passed TRUE,
   a comment is placed in the assembly code.  This function is used, for
   example, to discard the return value in an assignment statement or
   function call
*/
void b_internal_pop (BOOLEAN display_flag);


/***** Nullary operators (zero items popped) *****/

/* b_duplicate pushes a duplicate of the datum currently on the stack.
   The datum is assumed to be of the given type.
*/
void b_duplicate (TYPETAG type);

/* b_push_ext_addr accepts a global variable name and emits 
   code to push the address of that variable onto the stack.
*/
void b_push_ext_addr (char *id);

/* Added to unify assignment of local and global variables. -SF 2/3/96 */
/* b_push_loc_addr accepts an offset value (from the frame pointer)
   as parameter, and emits code to push the effective address offset(%ebp)
   onto the stack.  This is what you would call to get the actual address
   of a parameter or local variable onto the stack, given the offset
   value for the variable.
*/
void b_push_loc_addr (int offset);

/* b_push_const_int accepts an integer value and emits code to
   push that value onto the stack.
*/
void b_push_const_int (int value);

/* b_push_const_double accepts a double value and emits code to
   push that value onto the stack.  It does this by generating a new 
   label for the value, inserting a labeled double value (two .long's)
   into the .rodata section, and pushing the 8-byte value at the label 
   onto the stack.
*/
void b_push_const_double (double value);

/* b_push_const_string accepts a string and emits code to "push
   the string onto the stack."  It does this by generating a new 
   label for the string, inserting a labeled line of ascii text 
   into the .rodata section, and pushing the address of the label 
   onto the stack.
*/
void b_push_const_string (char *string);


/***** Unary operators (one item popped) *****/

/* b_offset accepts an offset value as a parameter, and assumes some
   address is currently on the stack.  It pops the address and pushes
   the result obtained by adding the offset to the address.  This is
   useful both for finding members in structs and for following reference
   links in Pascal.
*/
void b_offset (int offset);

/* b_deref accepts a type.  It assumes that the address of
   a variable of that type is on the stack.  It pops the
   address and pushes the value stored at that address
   onto the stack.
*/
void b_deref (TYPETAG type);

/* b_convert accepts a from_type and a to_type and emits code to
   convert a value of type from_type to a value of type to_type.
   It assumes that there is a value of type from_type on the 
   stack.  That value is popped off the stack, converted to a value 
   of the to_type, and pushed back onto the stack.
*/
void b_convert (TYPETAG from_type, TYPETAG to_type);

/* b_negate accepts a type and emits code to negate a value of 
   that type.  It assumes a value of that type is on the stack.
   It pops that value off the stack, negates it, and pushes it
   back onto the stack.
*/
void b_negate (TYPETAG type);

/* Changed to treat uniformly global and local variables, and to
   allow arbitrary l-values (not just id's).  -SF 2/3/96 */
/* b_inc_dec accepts a type, an increment-decrement operator
   (B_PRE_INC, B_POST_INC, B_PRE_DEC, B_POST_DEC), and a size
   parameter.  It emits code to do the indicated
   increment-decrement operation on a variable of the indicated
   type.  It is assumed that a pointer (l-value) is on top of
   the stack.  The function emits code to pop the pointer off the
   stack, increment/decrement the variable pointed to (which is assumed
   to have the given type), then pushes the value (r-value!) of the
   variable back on the stack.  The value pushed is that of the variable
   either before or after the inc/dec, depending on which operator
   was used.

   The size parameter is ignored unless type is TYPTR, in which case
   size should be the size (in bytes) of a datum pointed to by a
   pointer of this type.
*/
void b_inc_dec (TYPETAG type, B_INC_DEC_OP idop, unsigned int size);


/***** Binary operators (two items popped) *****/

/* b_assign accepts a type and emits code to store a value of that
   type in a variable OF THE SAME TYPE.  It assumes that a value of 
   that type is at the top of the stack and that the address of the 
   variable is the next item on the stack.  It pops both items off 
   the stack, stores the value at the address, AND PUSHES THE VALUE
   BACK ONTO THE STACK.  In other words, this is the code you would need 
   for assigning a value to a variable in C.  Note that it is assumed that
   the stack contains the actual address of the object, so for local
   variables and parameters, you must obtain the actual address beforehand
   using b_push_loc_addr().  To do an an assignment in Pascal, which does
   not use the value, follow b_assign() with b_pop().
*/
void b_assign (TYPETAG type);

/* b_arith_rel_op accepts a binary arithmetic or relational operator
   and a type.  The operators are:

        B_ADD       add (+)
	B_SUB       substract (-) 
	B_MULT      multiply (*)
	B_DIV       divide (/)
	B_MOD       mod (%)
	B_LT        less than (<)
	B_LE        less than or equal to (<=)
	B_GT        greater than (>)
	B_GE        greater than or equal to (>=)
	B_EQ        equal (==)
	B_NE        not equal (!=)
   
   It assumes that two values of the indicated type are on the 
   stack.  It pops those values off the stack, performs the 
   indicated operation, and pushes the resulting value onto
   the stack.

   No arithmetic on pointers is allowed in this function,
   although pointer comparisons are okay.  For pointer arithmetic,
   use b_ptr_arith_op.

   NOTE:  For arithmetic operators that are not commutative, it
          assumes that the operands were pushed onto the stack
	  in left-to-right order (e.g. if the expression is
	  x - y, y is at the top of the stack and x is the 
	  next item below it.

   NOTE:  For relational operators, a value of either 1 (true)
          or 0 (false) is pushed onto the stack.
*/
void b_arith_rel_op (B_ARITH_REL_OP arop, TYPETAG type);


/*****                                                                *****
 ***** Function defn, fcn call, local vars, param handling routines   *****
 *****                                                                *****/

/* b_func_prologue accepts a function name and generates the prologue
   for a function with that name.  It also initializes four static
   variables that are used in b_store_formal_param.
*/
void b_func_prologue (char *f_name);

/* b_init_formal_param_offset does the same thing as b_func_prologue,
   but only initializes the offset variables and emits no assembly code.
*/
void b_init_formal_param_offset ();

/* b_func_epilogue accepts the name of a function and emits code
   for the epilogue of a function by that name.
*/
void b_func_epilogue (char *f_name);

/* (Pascal only)
   b_set_return copies the value currently on the stack into the space
   designated for the return value, which should not be TYVOID.  The space
   designated for the return value is given by %ebp + return_value_offset.
   The return_type argument is the return type of the function.
   b_set_return assumes that the type of the value currently on the stack
   is the same as return_type, and this value is popped after it is copied.

   This is only needed for Pascal functions, which can assign and update
   a return value anywhere, any number of times.
*/
void b_set_return (TYPETAG return_type);

/* (Pascal only)
   b_prepare_return prepares for a return from a Pascal function or Pascal
   procedure.  The type argument is the return type of the function
   (TYVOID for a procedure).  Does nothing if TYVOID; otherwise assumes the
   value to be returned is at %ebp + return_value_offset, and that its type
   is the same as return_type.  It loads this return value into the
   proper return register.

   This function should be called once for each function body, right before
   calling b_func_epilogue(), which does the actual return.
*/
void b_prepare_return (TYPETAG return_type);

/* b_store_formal_param accepts the type of a parameter.  It must be called
   for each formal parameter, immediately after b_func_prologue.  It determines
   what register (if any) that parameter must be in, and emits code to
   move that parameter from the register to the stack at the appropriate
   offset relative to the frame pointer.  base_offset and double_base_offset
   are static variables maintained by the backend.  The function returns the
   offset (from the frame pointer) at which this
   parameter should be stored.  (For the curious, base_offset and
   double_base_offset get initialized in b_func_prologue.)

   For example, if your function has 3 parameters (int, int, double)
   this function would be called as follows:

       offset = b_store_formal_param (TYSIGNEDINT);
       offset = b_store_formal_param (TYSIGNEDINT);
       offset = b_store_formal_param (TYDOUBLE);

   Note that a call to b_store_formal_param must be made for each formal
   parameter in left-to-right order on the parameter list.

   All necessary type conversions are performed on the argument values,
   so, for example, calling

       offset = b_store_formal_param (TYFLOAT);
       offset = b_store_formal_param (TYSIGNEDCHAR);

   will automatically convert the first argument from double to float,
   and the second argument from int to char.

   Reference parameters (or "var" parameters in Pascal) are l-values, and
   therefore should always be stored using TYPTR, regardless of their
   actual type.
*/
int b_store_formal_param (TYPETAG type);

/* b_get_formal_param_offset does the same thing as b_store_formal_param
   except that it only updates the offset variables and returns the offset
   of the parameter, without generating any assembly code.
*/
int b_get_formal_param_offset (TYPETAG type);

/* (Pascal only)
   b_alloc_return_value allocates on the stack space to hold the return
   value for the current function.  This is only required for Pascal
   functions, where the return value can be set/updated any number of
   times and must persist across proc/func calls.

   Allocates 8 bytes and sets the global return_value_offset to the
   allocated space.
*/
void b_alloc_return_value();

/* b_alloc_local_vars accepts an integer and emits code to increase the
   stack by that number of bytes, adjusted upward to maintain quadword
   (8-byte) alignment of %esp.  This function should be used to allocate
   space for all the variables in the declaration section of a block
   (not necessarily all at once).  The size passed to b_alloc_local_vars
   must be at least the sum of the sizes (in bytes) of the variables,
   including any padding necessary for alignment.  The offset (from %ebp)
   of the variable with lowest address is returned.

   NOTE: Passing a size that is bigger than necessary is unlikely to
   cause any runtime errors; it merely leaves unused space on the stack
   for the duration of the function execution.
*/
int b_alloc_local_vars (int size);

/* (Pascal only)
   b_get_local_var_offset returns the current value of loc_var_offset.
   In Pascal, local variable offsets must be computed long before space
   for them is actually allocated, so this function can be called once
   after all formal parameter offsets have been computed (using
   b_get_formal_param_offset), but before local variables are declared.
   If properly initialized (either by calling b_init_formal_param_offset()
   or b_func_prologue()), the offset returned by b_get_local_var() is
   always <= 0 and eight-byte aligned, and marks the point below which local
   variables may be allocated.  A positive return value indicates a lack of
   proper initialization, and serves as a bug check for your code.
   For a non-void function, the first eight-bytes (with offset
   b_get_local_var_offset() - 8) should be reserved for the return value of
   the function.  b_get_local_var_offset is completely passive--it emits no
   assembly code and has no effect on state variables.
*/
int b_get_local_var_offset();

/* b_dealloc_local_vars accepts an integer and emits code to decrease the
   stack by that number of bytes.  The stack pointer is restored
   (if necessary) to quadword (8-byte) alignment.  The size value passed
   in must match the size passed to b_alloc_local_vars() at the beginning
   of the block.

   NOTE: Not calling this function at the end of a block is unlikely to
   cause any runtime errors; it merely leaves unused space on the stack
   for the duration of the function execution.
*/
void b_dealloc_local_vars (int size);

/* b_alloc_arglist takes the total size (in bytes) of actual arguments in a
   function call, and allocates space on the stack for the actual argument
   values of the function call.  It also pushes and initializes
   actual_arg_word_count[aaa_top] to 0.  This value is used to count the
   words used by the actual arguments to the function, in order to determine
   register storage of actual parameters.  (The value is used and updated
   in b_load_arg).  A word is 4 bytes.

   NOTE: you must call b_alloc_arglist for each function call, even if
   no actual arguments are passed.  Also, every call to b_alloc_arglist
   must be followed (as in matching parentheses) by a call to either
   b_funcall_by_name or b_funcall_by_ptr, with zero or more calls to
   b_load_arg in between.
*/
void b_alloc_arglist (int total_size);

/* b_load_arg accepts the type of an argument in a function call,
   and assumes that a value of this type is on top of the stack.
   It determines where in the argument list to put the argument:
   whether the argument should be in a register or on the stack.

   If the argument should be in a register, it emits code to move 
   the value of that argument (assumed to be at the top of the
   stack and of the proper type) from the stack to the proper 
   register.

   If the argument should be in the stack portion of the argument list,
   it emits code to move the value of that argument (assumed to be at the
   top of the stack and of the proper type) from the stack to another
   location in the stack (which has already been allocated by
   b_alloc_arglist) that will be the proper offset from the new
   frame pointer when control is transferred to the function.

   The word-offset of the argument is initialized in b_alloc_arglist,
   and is increased with each call to b_load_arg.

   For example, if your function is named "foo" with return type float
   and 3 actual parameters (int, double, double) this function would be
   called as follows:

       b_alloc_arglist(20);	// 20 bytes total - higher values are OK

       [here, put code to push the value of arg1 onto the stack]

       b_load_arg (TYSIGNEDINT);

       [here, put code to push the value of arg2 onto the stack]

       b_load_arg (TYDOUBLE);

       [here, put code to push the value of arg3 onto the stack]

       b_load_arg (TYDOUBLE);

       b_funcall_by_name ("foo", TYFLOAT);

   WARNING: it is assumed that the argument list lies direcly underneath the
   current top value on the stack.  Therefore, the sequence above must be
   followed strictly: b_load_arg is called after each push of an argument
   value, before the next argument is pushed.

   The only exception to this regimen is if a argument is being passed by
   reference (that is, it corresponds to a C++ reference parameter or
   (Pascal) VAR parameter of the function), b_load_arg is called with TYPTR,
   regardless of the actual type of the argument. (Such an argument must be
   an l-value, so a pointer value is on top of the stack.)

   Note also that b_load_arg does NOT leave the value of the argument on
   the stack, i.e., the value is popped.
*/
void b_load_arg (TYPETAG type);

/* b_funcall_by_name accepts a function name and a
   return type for the function.  It emits code to jump to 
   that function, pop any space off the stack used for actual
   arguments upon returning from the function, and push the return
   value (if any) of the function onto the stack.  Uses
   actual_arg_word_count[aaa_top] to find the amount of space taken by
   the arguments, and pops this value off of the actual_arg_word_count
   stack.  If the return type is TYVOID, then nothing is pushed on the
   stack upon return.

   Both b_funcall_by_name and b_funcall_by_ptr should be used in
   conjunction with the routines b_alloc_arglist and b_load_args.
   Each call to b_funcall_by_name or b_funcall_by_ptr must be preceded
   (as with matching parentheses) by a call to b_alloc_arglist, with
   some number of calls (possibly zero) to b_load_args in between.

   b_funcall_by_name and b_funcall_by_ptr differ in only one way: the
   former requires an explicit function name as argument, while the
   latter assumes the entry address of the function has been pushed
   onto the stack at runtime.
*/
void b_funcall_by_name (char *f_name, TYPETAG return_type);



/**************************
 *                        *
 * Routines for Project 3 *
 *                        *
 **************************/


/* b_label emits a label
*/
void b_label (char *label);

/* new_symbol generates unique symbols that can be used as 
   labels in the assembly code being emitted. The string returned
   is guaranteed to be distinct from any string returned from a
   previous call to new_symbol(), as well as any Pascal identifier.
*/
char *new_symbol ();

/* b_jump accepts a label and emits an unconditional jump to
   that label.
*/
void b_jump (char *label);

/* b_cond_jump accepts a TYPETAG, a B_COND (B_ZERO or B_NONZERO),
   and a label.  It assumes that there is a value of type "type"
   on the stack and emits code that pops the value off the stack
   and does a conditional jump based on that value and the B_COND
   supplied.  For example, calling b_cond_jump with the arguments
   TYSIGNEDINT, B_ZERO, and ".L1" generates code that pops an 
   integer off the stack and checks the value.  If it is zero it
   jumps to ".L1", otherwise it does not jump.

   Note:  The function "new_symbol" is the source of new labels.
          Every time you call new_symbol you get a new label.

   Note:  See function b_dispatch for a different type of
          conditional jump.
*/
void b_cond_jump (TYPETAG type, B_COND cond, char *label);

/* b_dispatch accepts a relational operator, a type, an integer
   comparison value, and a label.  The operator must be either B_EQ,
   B_NE, B_LT, B_LE, B_GT, or B_GE.  The type must be either
   TYSIGNEDINT, TYUNSIGNEDINT, TYSIGNEDLONGINT, or TYUNSIGNEDLONGINT.
   It assumes that there is a value on the stack of the given type,
   and emits code that compares that value (left) with the cmp_value
   (right).  If the two values do NOT satisfy the relation, then no
   jump is executed and the stack value is left on the stack.  If the
   two values DO satisfy the relation, then the stack value is either
   popped off the stack (if pop_on_jump) or left on the stack (if not
   pop_on_jump), and the jump is executed.  For example, calling
   b_dispatch with the arguments B_LE, TYSIGNEDINT, 45, ".L1", and TRUE
   generates code that compares the integer value v on the top of the
   stack with 45.  If v > 45, then no jump is executed and v is left on
   the stack.  If v <= 45, then v is popped off the stack and a jump to
   ".L1" occurs.

   Note:  The function "new_symbol" is the source of new labels.
          Every time you call new_symbol you get a new label. 

   Note:  See function b_cond_jump for a different type
          of conditional jump.
*/
void b_dispatch (B_ARITH_REL_OP op, TYPETAG type, int cmp_value, char *label,
		 BOOLEAN pop_on_jump);

/* b_encode_return encodes a return statement in a function.  The type
   argument is the type of the return expression (after assignment
   conversion to the return type of the function) if there is one.  If
   there is no return expression, TYVOID should be passed as the
   argument.  Assumes the value to be returned is on top of the stack,
   and that its type is the same as return_type.

   This function is not needed for compiling standard Pascal.
*/
void b_encode_return (TYPETAG return_type);



/**************************
 *                        *
 * Routines for Project 4 *
 *                        *
 **************************/


/* b_ptr_arith_op takes an operator (which must be either B_ADD or B_SUB),
   the type of the second argument, and the size of object pointed to
   by the pointer argument(s).  It assumes that two values are on the
   stack: a pointer value (to an object of size size) as first argument,
   and either a pointer or int as second argument (specified by type).
   The second argument is on top of the stack.  b_ptr_arith_op pops the
   two arguments off the stack, performs the given pointer arithmetic
   operation, and pushes the result back on the stack.

   Only legal operations are performed, e.g., adding two pointers is not
   allowed.  The resulting value is a pointer, unless two pointers are
   subtracted, in which case the result is an integer.

   Note: this function does not handle pointer comparisons.  That is
   done in b_arith_rel_op.
*/
void b_ptr_arith_op (B_ARITH_REL_OP arop, TYPETAG type, unsigned int size);

/* b_funcall_by_ptr accepts the return type for a function, and when
   called, assumes that the entry address of the function is on top
   of the stack.  It emits code to pop the entry address and jump to 
   that function, then upon return, pop any space for actual arguments
   used by function, then finally push the return value (if any) of 
   the function onto the stack.  Uses actual_no[aaa_top] to find
   the number of actual arguments of the call, and pops this
   value off of actual_no stack.  The entry address of the function
   gets popped in the process.  If the return type
   is TYVOID, then nothing is pushed on the stack upon return.

   Both b_funcall_by_name (Project 2) and b_funcall_by_ptr should be used
   in conjunction with the routines b_alloc_arglist and b_load_args.
   Each call to b_funcall_by_name or b_funcall_by_ptr must be preceded
   (as with matching parentheses) by a call to b_alloc_arglist, with
   zero or more calls to b_load_args in between.

   b_funcall_by_name and b_funcall_by_ptr differ in only one way: the
   former requires an explicit function name as argument, while the
   latter assumes the entry address of the function has been pushed
   onto the stack.
*/
void b_funcall_by_ptr (TYPETAG return_type);



/**************************
 *                        *
 * Miscellaneous routines *
 *                        *
 **************************/


/*  emit prints printf strings to outfp with an end-of-line character
 *  appended to the end.
*/
void emit( char *format, ... );

/*  emitn prints printf strings to outfp with no end-of-line character
*/ 
void emitn( char *format, ... );

/* b_lineno_comment generates a comment in the assembly code, displaying
   the given number.  It should be called from scan.l to generate the number
   of the new line as soon as a '\n' is detected in the source file.
*/
void b_lineno_comment (int lineno);

/* b_arith_rel_op_string accepts an arithmetic/relational
   operator of type B_ARITH_REL_OP and returns a string
   indicating the nature of the arithmetic or relational
   operator.
*/
char *b_arith_rel_op_string (B_ARITH_REL_OP arop);

/* my_print_typetag is a version of ty_print_typetag (found
   in types.c) that sends its output to stdout instead of
   stderr.
*/
void my_print_typetag (TYPETAG tag);

#endif
