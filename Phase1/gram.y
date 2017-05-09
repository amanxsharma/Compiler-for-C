/*
 *
 * yacc/bison input for simplified C++ parser
 *
 */

%{

#include "defs.h"
#include "types.h"
#include "symtab.h"
#include "bucket.h"
#include "message.h"
#include "encode.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int type_flag = 0;
ST_DR st_dr;
ST_ID st_id, temp_st_id;
TYPE type;
int flag_for_function_st_id = 0;
PARAM_LIST param_list;
BOOLEAN flag_is_ref = FALSE;

/********************************************/
#include "tree.h"
/********************************************/

    int yylex();
    int yyerror(char *s);

%}

%union {
	int	y_int;
	double	y_double;
	char *	y_string;
	struct IdentifierTree* y_tree;
	struct bucket* y_bucket;
	TYPE y_type;
	TYPE_SPECIFIER y_type_specifier;
	struct expression* y_expr;
	struct param* y_param;
	};
	
	
/*********************************************/

%token <y_string> IDENTIFIER
%token <y_int> INT_CONSTANT
%token <y_double> DOUBLE_CONSTANT
%token <y_string> STRING_LITERAL 
%type <y_string> pointer
%type <y_tree> identifier direct_declarator declarator init_declarator init_declarator_list constant_expr conditional_expr logical_or_expr logical_and_expr
%type <y_bucket> declaration declaration_specifiers type_specifier storage_class_specifier type_qualifier
%type <y_bucket> TYPEDEF EXTERN STATIC AUTO REGISTER VOID CONST VOLATILE ENUM enum_specifier struct_or_union_specifier TYPE_NAME struct_or_union struct_declaration_list
%type <y_bucket> enumerator_list STRUCT UNION struct_declaration enumerator specifier_qualifier_list
%type <y_tree> inclusive_or_expr exclusive_or_expr and_expr equality_expr relational_expr shift_expr additive_expr multiplicative_expr cast_expr


/*********************************************/

%token SIZEOF
%token PTR_OP INC_OP DEC_OP LEFT_OP RIGHT_OP LE_OP GE_OP EQ_OP NE_OP
%token AND_OP OR_OP MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN
%token SUB_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN
%token XOR_ASSIGN OR_ASSIGN TYPE_NAME

%token TYPEDEF EXTERN STATIC AUTO REGISTER
%token CHAR SHORT INT LONG SIGNED UNSIGNED FLOAT DOUBLE CONST VOLATILE VOID
%token STRUCT UNION ENUM ELIPSIS

%token CASE DEFAULT IF ELSE SWITCH WHILE DO FOR GOTO CONTINUE BREAK RETURN

%token BAD

%start translation_unit
%%

 /*******************************
  * Expressions                 *
  *******************************/

primary_expr
	: identifier
	| INT_CONSTANT			{$<y_int>$ = update_expr_int($1);}
	| DOUBLE_CONSTANT		{$<y_double>$ = update_expr_double($1);}
	| STRING_LITERAL
	| '(' expr ')'
	;

postfix_expr
	: primary_expr
	| postfix_expr '[' expr ']'
	| postfix_expr '(' argument_expr_list_opt ')'
	| postfix_expr '.' identifier
	| postfix_expr PTR_OP identifier
	| postfix_expr INC_OP
	| postfix_expr DEC_OP
	;

argument_expr_list_opt
	: /* null derive */
	| argument_expr_list
	;

argument_expr_list
	: assignment_expr
	| argument_expr_list ',' assignment_expr
	;

unary_expr
	: postfix_expr
	| INC_OP unary_expr
	| DEC_OP unary_expr
	| unary_operator cast_expr
	| SIZEOF unary_expr
	| SIZEOF '(' type_name ')'
	;

unary_operator
	: '&' | '*' | '+' | '-' | '~' | '!'
	;

cast_expr
	: unary_expr
	| '(' type_name ')' cast_expr
	;

multiplicative_expr
	: cast_expr
	| multiplicative_expr '*' cast_expr
	| multiplicative_expr '/' cast_expr
	| multiplicative_expr '%' cast_expr
	;

additive_expr
	: multiplicative_expr
	| additive_expr '+' multiplicative_expr
	| additive_expr '-' multiplicative_expr
	;

shift_expr
	: additive_expr
	| shift_expr LEFT_OP additive_expr
	| shift_expr RIGHT_OP additive_expr
	;

relational_expr
	: shift_expr
	| relational_expr '<' shift_expr
	| relational_expr '>' shift_expr
	| relational_expr LE_OP shift_expr
	| relational_expr GE_OP shift_expr
	;

equality_expr
	: relational_expr
	| equality_expr EQ_OP relational_expr
	| equality_expr NE_OP relational_expr
	;

and_expr
	: equality_expr
	| and_expr '&' equality_expr
	;

exclusive_or_expr
	: and_expr
	| exclusive_or_expr '^' and_expr
	;

inclusive_or_expr
	: exclusive_or_expr
	| inclusive_or_expr '|' exclusive_or_expr
	;

logical_and_expr
	: inclusive_or_expr
	| logical_and_expr AND_OP inclusive_or_expr
	;

logical_or_expr
	: logical_and_expr
	| logical_or_expr OR_OP logical_and_expr
	;

conditional_expr
	: logical_or_expr
	| logical_or_expr '?' expr ':' conditional_expr
	;

assignment_expr
	: conditional_expr
	| unary_expr assignment_operator assignment_expr
	;

assignment_operator
	: '=' | MUL_ASSIGN | DIV_ASSIGN | MOD_ASSIGN | ADD_ASSIGN | SUB_ASSIGN
	| LEFT_ASSIGN | RIGHT_ASSIGN | AND_ASSIGN | XOR_ASSIGN | OR_ASSIGN
	;

expr
	: assignment_expr
	| expr ',' assignment_expr
	;

constant_expr
	: conditional_expr
	;

expr_opt
	: /* null derive */
	| expr
	;

 /*******************************
  * Declarations                *
  *******************************/

declaration
	: declaration_specifiers ';'						{error("no declarator in declaration");}	
	| declaration_specifiers  init_declarator_list ';' 
	;
	

declaration_specifiers
	: storage_class_specifier
	| storage_class_specifier declaration_specifiers
	| type_specifier									{$<y_bucket>$ = update_bucket(NULL, $<y_type_specifier>1, NULL);}
	| type_specifier declaration_specifiers				{$<y_bucket>$ = update_bucket($<y_bucket>2, $<y_type_specifier>1, NULL);}
	| type_qualifier
	| type_qualifier declaration_specifiers
	;

init_declarator_list
	: init_declarator 									{st_dr = stdr_alloc(); 
														st_dr->tag = GDECL; 
														st_dr->u.decl.type = build_id(build_base($<y_bucket>0), $<y_tree>1);;
														TYPETAG t = ty_query(st_dr->u.decl.type);
														st_dr->u.decl.sc = NO_SC;
														BOOLEAN flag = st_install(st_id, st_dr);
														if(flag == FALSE)
														{error("duplicate declaration for %s",st_get_id_str(st_id));
														error("duplicate definition of `%s'",st_get_id_str(st_id));}
														if(t != TYFUNC && flag == TRUE){
														encode(st_dr->u.decl.type, st_id);
														}}
														
	| init_declarator_list ',' init_declarator 			{st_dr = stdr_alloc(); 
														st_dr->tag = GDECL; 
														st_dr->u.decl.type = build_id(build_base($<y_bucket>0), $<y_tree>3);;
														TYPETAG t = ty_query(st_dr->u.decl.type);
														st_dr->u.decl.sc = NO_SC;
														BOOLEAN flag = st_install(st_id, st_dr);
														if(flag == FALSE)
														{error("duplicate declaration for %s",st_get_id_str(st_id));
														error("duplicate definition of `%s'",st_get_id_str(st_id));}
														if(t != TYFUNC && flag == TRUE){
														encode(st_dr->u.decl.type, st_id);
														}}
	;

init_declarator
	: declarator 
	| declarator '=' initializer
	;

storage_class_specifier
	: TYPEDEF | EXTERN | STATIC | AUTO | REGISTER
	;

type_specifier
	: VOID 		{$<y_type_specifier>$ = VOID_SPEC;}
	| CHAR 		{$<y_type_specifier>$ = CHAR_SPEC;}
	| SHORT 	{$<y_type_specifier>$ = SHORT_SPEC;}
	| INT   	{$<y_type_specifier>$ = INT_SPEC;}
	| LONG  	{$<y_type_specifier>$ = LONG_SPEC;}
	| FLOAT     {$<y_type_specifier>$ = FLOAT_SPEC;}
	| DOUBLE    {$<y_type_specifier>$ = DOUBLE_SPEC;}
	| SIGNED    {$<y_type_specifier>$ = SIGNED_SPEC;}
	| UNSIGNED  {$<y_type_specifier>$ = UNSIGNED_SPEC;}
	| struct_or_union_specifier
	| enum_specifier
	| TYPE_NAME
	;

struct_or_union_specifier
	: struct_or_union '{' struct_declaration_list '}'
	| struct_or_union identifier '{' struct_declaration_list '}'
	| struct_or_union identifier
	;

struct_or_union
	: STRUCT
	| UNION
	;

struct_declaration_list
	: struct_declaration
	| struct_declaration_list struct_declaration
	;

struct_declaration
	: specifier_qualifier_list struct_declarator_list ';'
	;

specifier_qualifier_list
	: type_specifier specifier_qualifier_list_opt
	| type_qualifier specifier_qualifier_list_opt
	;

specifier_qualifier_list_opt
	: /* null derive */					
	| specifier_qualifier_list			
        ;

struct_declarator_list
	: struct_declarator
	| struct_declarator_list ',' struct_declarator
	;

struct_declarator
	: declarator
	| ':' constant_expr
	| declarator ':' constant_expr
	;

enum_specifier
	: ENUM '{' enumerator_list '}'
	| ENUM identifier '{' enumerator_list '}'
	| ENUM identifier
	;

enumerator_list
	: enumerator
	| enumerator_list ',' enumerator
	;

enumerator
	: identifier	{}
	| identifier '=' constant_expr	{}
	;

type_qualifier
	: CONST | VOLATILE
	;

declarator
	: direct_declarator									
	| pointer declarator								{	$<y_tree>$ = push($<y_string>1,"*",$<y_tree>2);
														}
	;		

direct_declarator
	: identifier
	| '(' declarator ')'	{$$ = $2;}
	| direct_declarator '[' ']'
	| direct_declarator '[' constant_expr ']'														{char* temp = (char*)malloc(sizeof(char)); 
																									sprintf(temp, "%d", $<y_int>3); 
																									$<y_tree>$ = push("array",temp,$<y_tree>1);}
	
	| direct_declarator '(' {flag_for_function_st_id =1; } 		parameter_type_list ')' 			{param_list = reverse(param_list); 
																									flag_for_function_st_id = 0;
																									$<y_tree>$ = push("function","func",$<y_tree>1); 
																									param_list = NULL; 
																					 				}
																					
	| direct_declarator '(' ')'																		{$<y_tree>$ = push("function","func",$<y_tree>1); }
	;

pointer
	: '*' specifier_qualifier_list_opt					{$<y_string>$ = "pointer";}
        | '&'											{flag_is_ref = TRUE; $<y_string>$ = "reference";}
	;

parameter_type_list
	: parameter_list
	| parameter_list ',' ELIPSIS
	;

parameter_list
	: parameter_declaration							
	| parameter_list ',' parameter_declaration		
	;

parameter_declaration
	: declaration_specifiers declarator				{param_list = create_param_list($<y_bucket>$, $<y_tree>2, temp_st_id, param_list, flag_is_ref); flag_is_ref = FALSE;}
	| declaration_specifiers						{error("no id in parameter list");}
	| declaration_specifiers abstract_declarator
	;

type_name
	: specifier_qualifier_list
	| specifier_qualifier_list abstract_declarator
	;

abstract_declarator
	: pointer
	| direct_abstract_declarator
	| pointer abstract_declarator
	;

direct_abstract_declarator
	: '(' abstract_declarator ')'
	| '[' ']'
	| '[' constant_expr ']'
	| direct_abstract_declarator '[' ']'
	| direct_abstract_declarator '[' constant_expr ']'
	| '(' ')'
	| '(' parameter_type_list ')'
	| direct_abstract_declarator '(' ')'
	| direct_abstract_declarator '(' parameter_type_list ')'
	;

initializer
	: assignment_expr
	| '{' initializer_list comma_opt '}'
	;

comma_opt
	: /* Null derive */
	| ','
	;

initializer_list
	: initializer
	| initializer_list ',' initializer
	;

 /*******************************
  * Statements                  *
  *******************************/

statement
	: labeled_statement
	| compound_statement
	| expression_statement
	| selection_statement
	| iteration_statement
	| jump_statement
	;

labeled_statement
	: identifier ':' statement
	| CASE constant_expr ':' statement
	| DEFAULT ':' statement
	;

compound_statement
	: '{' '}'
	| '{' statement_list '}'
	| '{' declaration_list '}'
	| '{' declaration_list statement_list '}'
	;

declaration_list
	: declaration
	| declaration_list declaration
	;

statement_list
	: statement
	| statement_list statement
	;

expression_statement
	: expr_opt ';'
	;

selection_statement
	: IF '(' expr ')' statement
	| IF '(' expr ')' statement ELSE statement
	| SWITCH '(' expr ')' statement
	;

iteration_statement
	: WHILE '(' expr ')' statement
	| DO statement WHILE '(' expr ')' ';'
	| FOR '(' expr_opt ';' expr_opt ';' expr_opt ')' statement
	;

jump_statement
	: GOTO identifier ';'
	| CONTINUE ';'
	| BREAK ';'
	| RETURN expr_opt ';'
	;

 /*******************************
  * Top level                   *
  *******************************/

translation_unit
	: external_declaration	{type_flag=0;}
	| translation_unit external_declaration
	;

external_declaration
	: function_definition
	| declaration
	;

function_definition
	: declarator compound_statement
	| declaration_specifiers declarator compound_statement
	;

 /*******************************
  * Identifiers                 *
  *******************************/

identifier
	: IDENTIFIER	{$<y_tree>$ = push("identifier",$1,NULL); 
					if(flag_for_function_st_id == 0)
					{st_id = st_enter_id($1);}
					else if(flag_for_function_st_id == 1)
					{temp_st_id = st_enter_id($1);}
					}
	;
%%

extern int column;

int yyerror(char *s)
{
	error("%s (column %d)",s,column);
        return 0;  /* never reached */
}


