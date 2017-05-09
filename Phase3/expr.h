/****************************************************************************/
/* 
	definitions to support expr.c
*/

#include "bucket.h"
#include "types.h"

//possible type of nodes in an expression
typedef enum{
	ADD_OP, SUB_OP, MULT_OP, DIV_OP, EQUAL_OP, NOT_EQUAL_OP, EQUAL_TO_OP, GREATER_THAN_EQUAL_OP, LESS_THAN_EQUAL_OP, GREATER_THAN_OP, LESS_THAN_OP
}BIN_OP_TYPE;

typedef enum{
	UMINUS_OP, POST_INC_OP, POST_DEC_OP, PRE_INC_OP, PRE_DEC_OP
}UN_OP_TYPE;

typedef enum{
	INT_CONST, VARIABLE
}NULL_OP_TYPE;

typedef enum{
	BINOP_NODE, UNOP_NODE, INT_CONST_NODE, DOUBLE_CONST_NODE, STRING_CONST_NODE, VAR_NODE, DEREF_NODE, FUNCTION_NODE
}TAG_TYPE;
typedef struct er_dup{
	char *id;
	struct er_dup *next,*prev;	
}ER,*er;

//structure for expression type of node
typedef struct expr{
	
	TAG_TYPE tag;							//Tag or type of node
	TYPETAG type_of_node;
	
	union{
		
		union{
			int int_const_value;						//int constant
			double double_const_value;					//double constant
			char * string_const_value; 					//string constant
		}value;
		
		char * id;							//variable
		
		struct {							//unary node
			UN_OP_TYPE op;					//Unary Operator type
			//TYPETAG un_op_tag;				//Tag for value with negative sign for example for -3 it is TYSIGNEDINT, ignore this for variables
			struct expr *arg;				//single operator for unary operator
		}un_op;
		
		struct {							//binary node
			BIN_OP_TYPE op;					//Binary Operator Type
			struct expr *arg1, *arg2;		//two arguments for binary operator
		}bin_op;	
		
		struct{
			char * func_id;									//function name
			struct argument_expr* func_arg_expr;			//function argument list
			//TYPETAG func_tag;								//function tag
		}func;
		
		struct{
			//TYPETAG deref_tag;              //TYPETAG for deref			
			struct expr *arg;
		}deref;
	}u;
	
}EXPR_NODE, *EXPR;

//functions to build expression
EXPR make_deref_node(EXPR arg);
EXPR make_int_const_node(int val);
EXPR make_double_const_node(double val);
EXPR make_string_const_node(char* val);
EXPR make_var_node(char *id,TYPETAG tag);
EXPR make_function_node(char* id, TYPETAG tag, struct argument_expr* arg_expr);
EXPR make_unop_node(UN_OP_TYPE op, EXPR arg);
EXPR make_binop_node(BIN_OP_TYPE op, EXPR arg1, EXPR arg2);

//general usage function, get overall type from the two types passed
TYPETAG get_cast_typetag(TYPETAG tag1, TYPETAG tag2);

//Function to evaluate expression
BOOLEAN eval(EXPR expr);

//Install function for function definitions
//void install_function(ST_ID st_id, TYPE f_type, BUCKET_PTR bucket, tr tree, PARAM_LIST param_list);
void  convert(TYPETAG t1,TYPETAG t2,EXPR expr);


//functions for constant folding
EXPR constant_fold(EXPR expr);
double const_switch(double i,double j,BIN_OP_TYPE t);
EXPR to_convert_rhs(EXPR lhs,EXPR rhs);

/******************************************************* For 90 Percent **************************************/
typedef struct argument_expr{
	
	EXPR expr_node;							//EXPR node	
	struct argument_expr *next, *prev;		//Next and previous to connect
	
}ARG_EXPR_NODE, *ARG_EXPR;

ARG_EXPR create_argument_expr_list(EXPR expr, struct argument_expr* arg_expr);
void print_argument_expr_list(ARG_EXPR arg_expr);
ARG_EXPR reverse_arg_expr_list(ARG_EXPR arg_expr);
int get_num_arg_expr_list(ARG_EXPR arg_expr);
void func_b_load_arg(ARG_EXPR arg_expr);
void call_b_store_formal_param(PARAM_LIST param_list, ST_ID st_id);

/**************for phase 3****************/

typedef enum{
	WHILE_LP,DOWHILE_LP,FOR_LP,SWITCH_ST	
}CTRL_TYPE;

/** to store the stack of exit label*****/
typedef struct  case_label_list
			{
				char *label;
				int const_val;
				BOOLEAN is_default;
				struct case_label_list *next_lbl,*prev_lbl;							
			}cs_lbl_l,*CS_LBL;
			
typedef struct label_stack
{
	CTRL_TYPE con_type;
	char * lbl_name;
	CS_LBL cl;
	struct label_stack *next,*prev;
}lbl_stk_node,*LBL_STK;

LBL_STK pop_label(LBL_STK node);
LBL_STK push_label(LBL_STK node,CTRL_TYPE ctrl_type,char *label_name);
LBL_STK push_case_label(LBL_STK node,char *case_lbl,int val);
LBL_STK push_default_label(LBL_STK node,char *default_label);
CS_LBL push_cslbl_list(CS_LBL list,char *case_lbl,int val);
CS_LBL push_default_list(CS_LBL list,char *case_lbl);
void loop_over_cslbl_list(LBL_STK stack);
