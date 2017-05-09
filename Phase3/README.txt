Process C control statements.

Part 1: You must process if statements, while statements, break statements, and return statements. Be sure to do the appropriate semantic error checking: break cannot appear outside a while, etc. Consult the appropriate C references and your favorite compilers to determine what conversions are applied to the return expressions.

Part 2: You should also process for statements. Be sure to do any appropriate semantic error checking. The break statement should be implemented in the context of the for statement, too.

Part 3: You must also process switch statements. The only kind of case constant expression you need to support is a simple integer constant (no operators, not even unary minus). You should enforce a rule that the type of the switch expression must be some integer type (it is subject to the usual unary conversions). Be sure to do other appropriate semantic error checking: case constants must be unique within a switch, only one default label for a given switch, etc. The break statement should be implemented in the context of the switch statement, too.

Before starting on switch it is highly recommended that you first familiarize yourself with switch statement semantics. According to Harbison & Steele (5th ed.), Section 8.7, the switch statement is executed as follows:

The control expression (the expression after the keyword switch) is evaluated.
If the value of the control expression is equal to that of the constant expression in some case label belonging to the switch statement, then program control is transferred to the point indicated by that case label as if by a goto statement.
If the value of the control expressions is not equal to any case label, but there is a default label that belongs to the switch statement, then program control is tranferred to the point indicated by that default label.
If the value of the control expression is not equal to any case label and there is no default label, no statement of the body of the switch statement is executed; program control is tranferred to whatever follows the switch statement.
(Above, we say that a label belongs to a switch statement if it appears in the body of the switch statement but is not inside the body of any nested switch statement.) When control is transferred to a case or default label, execution continues through successive statements, ignoring any additional case or default labels that are encountered, until the end of the switch statement is reached or until control is transferred out of the switch statement by a break or return (or goto or continue) statement.
There are four primary functions in the back end that are needed for this assignment: b_jump, b_cond_jump, b_dispatch, and b_encode_return, and two additional supporting functions, b_label and new_symbol.

The function new_symbol returns a fresh, unique, saved string that can be used as a label and jump destination; b_label emits a given string as a label. Each call to new_symbol returns a different string, which is guaranteed not to conflict with any legal C identifier. The function b_jump generates an unconditional jump to a given label; b_cond_jump generates a conditional jump based on the value of the top of the stack (the value is also popped off the stack).

The function b_dispatch is to be used in handling a switch statement. It takes an integer argument n and generates code that compares n with the value on top of the stack (which must be an integer). If the two values are equal, a jump is executed and the value is popped, otherwise no jump occurs and the value is left on the stack. Thus, several calls to b_dispatch can be called in a row, each comparing the top of the stack with a series of different integer constants.

The function b_encode_return generates code to return from a function with an optional return value. If there is an expression following the return keyword, as in

    return x+3;

then the expression's value should be assignment-converted to the return type of the function, and this type should be passed to b_encode_return. The generated assembly code expects the converted value to be on top of the stack. If there is no expression after the return keyword, then TYVOID should be passed to b_encode_return, and the resulting code does not assume any value on top of the stack. (Note: an expressionless return in a function with a nonvoid return type is not a semantic error! The return value is undefined in this case.)