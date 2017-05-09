/*								*/
/*								*/
/*	CSCE 531 - "C++" compiler				*/
/*								*/
/*								*/
/*		Module Implementing "C/C++" Types		*/
/*								*/
/*	--expr.c--						*/
/*								*/
/*	
/*								*/
/*								*/
/*								*/
/****************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "defs.h"
#include "types.h"
#include "symtab.h"
#include "message.h"
#include "tree.h"
#include "bucket.h"
#include "expr.h"
#include "backend-x86.h"

BOOLEAN direct_assignment = FALSE; //this flag is turned TRUE if there is a direct assignment from float type to float or char to char
//example, float i,j; i = j; then we don't need to convert these values to double and then reconvert.
/* actual definition of expr */

/***************************CONSTANTS*****************************/
EXPR make_int_const_node(int val){
	EXPR ret = (EXPR) malloc(sizeof(EXPR_NODE));
	assert(ret!=NULL);
	
	ret->tag = INT_CONST_NODE;
	ret->type_of_node=TYSIGNEDINT;
	ret->u.value.int_const_value = val;
	
	return ret;
}

int get_expr_int(EXPR expr)
{
	return expr->u.value.int_const_value;
}

EXPR make_double_const_node(double val){
	EXPR ret = (EXPR) malloc(sizeof(EXPR_NODE));
	assert(ret!=NULL);
	
	ret->tag = DOUBLE_CONST_NODE;
	ret->type_of_node=TYDOUBLE;
	ret->u.value.double_const_value = val;
		
	return ret;
}

EXPR make_string_const_node(char* val){
	EXPR ret = (EXPR) malloc(sizeof(EXPR_NODE));
	assert(ret!=NULL);
	
	ret->tag = STRING_CONST_NODE;
	ret->type_of_node = TYPTR;
	ret->u.value.string_const_value = val;
	
	return ret;
}

/***************************BINARY OPERATOR*****************************/
EXPR make_binop_node(BIN_OP_TYPE op, EXPR arg1, EXPR arg2)
{
	EXPR ret = (EXPR) malloc(sizeof(EXPR_NODE));
	assert(ret!=NULL);
	
	ret->tag = BINOP_NODE;	//tag type
	ret->u.bin_op.op = op;	//operator type
	ret->u.bin_op.arg1 = arg1; //argument 1
	ret->u.bin_op.arg2 = arg2; //argument 2
	
	TYPETAG t1=arg1->type_of_node;
	TYPETAG t2=arg2->type_of_node;
	
	ret->type_of_node=get_cast_typetag(t1, t2);
	
	return ret;
}

/***************************UNARY OPERATOR*****************************/
EXPR make_unop_node(UN_OP_TYPE op, EXPR arg)
{
	EXPR ret = (EXPR) malloc(sizeof(EXPR_NODE));
	assert(ret!=NULL);
	
	ret->tag = UNOP_NODE;		//tag type
	ret->u.un_op.op = op;		//operator type
	ret->u.un_op.arg = arg;		//argument
	ret->type_of_node = arg->type_of_node;
	
	return ret;
}

/***************************VARIABLE NODE*****************************/
EXPR make_var_node(char *id,TYPETAG tag)
{
	EXPR ret = (EXPR) malloc(sizeof(EXPR_NODE));
	assert(ret!=NULL);
	
	ret->tag = VAR_NODE;	//tag type
	ret->type_of_node=tag;
	ret->u.id = id; 		//variable name
}

/***************************DEREF NODE*****************************/
EXPR make_deref_node(EXPR arg)
{

	EXPR ret = (EXPR) malloc(sizeof(EXPR_NODE));
	assert(ret!=NULL);
	
	ret->tag = DEREF_NODE;
	ret->type_of_node=arg->type_of_node;
	//ret->u.deref.deref_tag = tag;	//tag required to call deref function
	ret->u.deref.arg = arg;			//Link to identifier
}

/***************************FUNCTION NODE*****************************/
EXPR make_function_node(char* id, TYPETAG tag, struct argument_expr* arg_expr)
{
	EXPR ret = (EXPR) malloc(sizeof(EXPR_NODE));
	assert(ret!=NULL);
	
	ret->tag = FUNCTION_NODE;
	ret->u.func.func_id = id;
	ret->type_of_node=tag;
	ret->u.func.func_arg_expr = arg_expr;
	//ret->u.func.func_tag = tag;
	//printf("returning from make function node\n");
}

/***************************EVALUATE*****************************/
BOOLEAN eval(EXPR expr)
{	
	BOOLEAN pop_flag = TRUE;	//pop if this is true
	//double ret, arg_val, arg1_val, arg2_val;
	switch(expr->tag)
	{	
		case FUNCTION_NODE:
			{	
				if(expr->type_of_node == TYVOID) {pop_flag = FALSE;}	//Do not pop if return type is VOID
				
				//print_argument_expr_list(expr->u.func.func_arg_expr);
				
				if(expr->u.func.func_arg_expr == NULL)
				{
					//printf("if\n");
					b_alloc_arglist(0);	//If no parameters then allocate 0 Bytes
					b_funcall_by_name(expr->u.func.func_id,expr->type_of_node);
				}
				else
				{
					//printf("else....\n");
					b_alloc_arglist(get_num_arg_expr_list(expr->u.func.func_arg_expr)*8);	//If there are arguments, then get num of args and multiply by 8 to get the size
					//b_load_arg needs to be called for each argument
					
					func_b_load_arg(expr->u.func.func_arg_expr);		//external function to call b_load_arg for each argument
					
					b_funcall_by_name(expr->u.func.func_id,expr->type_of_node);
				}
				
				//printf("exit\n");
				break;
			}
		case VAR_NODE:
			{	
				ST_ID st_id = st_lookup_id(expr->u.id);
				int * block_num;
				ST_DR st_dr = st_lookup(st_id, &block_num);
				//printf("st_dr->tag = %d...st_dr->u.decl.binding = %d \n",st_dr->tag,st_dr->u.decl.binding);
				/*if(st_dr->tag != GDECL)
				{
					b_push_loc_addr(st_dr->u.decl.binding);
					if(st_dr->u.decl.is_ref == TRUE)		//create immediate deref node if it was passed as referenced in parameters in function definition
						b_deref(TYPTR);
				}
				else
				{
					b_push_ext_addr(expr->u.id);
				}*/
				
				if(st_dr->tag == GDECL)
					b_push_ext_addr(expr->u.id);
				else
					b_push_loc_addr(st_dr->u.decl.binding);
					if(st_dr->u.decl.is_ref == TRUE)		//create immediate deref node if it was passed as referenced in parameters in function definition
						b_deref(TYPTR);
			//	printf("variable = %s has is_ref = %d\n",expr->u.id,st_dr->u.decl.is_ref);
				
				
				break;
			}
		case INT_CONST_NODE:
			{	
				
				b_push_const_int(expr->u.value.int_const_value);
				break;
			}
		case DOUBLE_CONST_NODE:
			{	
				
				b_push_const_double(expr->u.value.double_const_value);
				break;
			}
		case STRING_CONST_NODE:
			{
			
				b_push_const_string(expr->u.value.string_const_value);
				break;
			}
		case DEREF_NODE:
			{	
				
				ST_ID st_id = st_lookup_id(expr->u.deref.arg->u.id);
				int * block_num;
				ST_DR st_dr = st_lookup(st_id, &block_num);
				if(st_dr->tag == GDECL)
					b_push_ext_addr(expr->u.deref.arg->u.id);
				else
				{
					b_push_loc_addr(st_dr->u.decl.binding);
					if(st_dr->u.decl.is_ref == TRUE)		//create immediate deref node if it was passed as referenced in parameters in function definition
						b_deref(TYPTR);
				}
					
					
			//	b_push_ext_addr(expr->u.deref.arg->u.id);
				b_deref(expr->type_of_node);
				
				if(typenum(expr->type_of_node) == 1)// && direct_assignment == FALSE)				//DEFAULT Conversion from CHAR to SIGNEDINT, only if it is not direct assignment
				{
						b_convert(expr->type_of_node, TYSIGNEDINT);
						expr->type_of_node = TYSIGNEDINT;
					//	printf("Inside if in deref node...converting CHAR\n");
				}
				if(typenum(expr->type_of_node) == 3)// && direct_assignment == FALSE)			    //DEFAULT conversion from Float to Double, only if it is not direct assignment
				{
					b_convert(expr->type_of_node, TYDOUBLE);
					expr->type_of_node = TYDOUBLE;
				//	printf("Inside if in deref node...CONVERTING FLOAT\n");
				}
				break;
			}
		case UNOP_NODE:
			{
				switch(expr->u.un_op.op)
				{
					case UMINUS_OP: 
						{							
							if(expr->u.un_op.arg->tag==INT_CONST_NODE)
							{
								EXPR temp=expr->u.un_op.arg;
								temp->u.value.int_const_value= -temp->u.value.int_const_value;
								eval(temp);	
							}
							else if(expr->u.un_op.arg->tag==DOUBLE_CONST_NODE)
							{
								EXPR temp=expr->u.un_op.arg;
								temp->u.value.double_const_value= -temp->u.value.double_const_value;
								eval(temp);		
							}
							else
							{
								eval(expr->u.un_op.arg);				//RECURSIVE CALLING
								b_negate(expr->type_of_node);
							}
							break;
						}
					case POST_INC_OP:
						{
							eval(expr->u.un_op.arg);
							b_inc_dec(expr->type_of_node, B_POST_INC, 8);
							break;
						}
					case POST_DEC_OP:
						{
							eval(expr->u.un_op.arg);
							b_inc_dec(expr->type_of_node, B_POST_DEC, 8);
							break;
						}
					case PRE_INC_OP:
						{
							eval(expr->u.un_op.arg);
							b_inc_dec(expr->type_of_node, B_PRE_INC, 8);
							break;
						}
					case PRE_DEC_OP:
						{
							eval(expr->u.un_op.arg);
							b_inc_dec(expr->type_of_node, B_PRE_DEC, 8);
							break;
						}
				}
				break;
			}
		case BINOP_NODE:
			{
				/*			DO NOT DELETE
				UNCOMMENT THIS IF LEFT NODE IS UNARY MINUS AND ISSUES EXIST WITH CONVERT AND DEREF
				if(expr->u.bin_op.arg1->tag == UNOP_NODE)
				{
					//printf("inside if for UNOP_NODE arg1 = %d, arg2 = %d\n",expr->u.bin_op.arg1->type_of_node,expr->u.bin_op.arg2->type_of_node);
					convert(expr->u.bin_op.arg1->type_of_node,expr->u.bin_op.arg2->type_of_node);
				}
				*/
				
				
				if(expr->u.bin_op.op == EQUAL_OP)
					direct_assignment = TRUE; //Global flag, read comments at the top for its description
				if((expr->u.bin_op.arg1->tag==UNOP_NODE)&&(expr->u.bin_op.arg1->u.un_op.op!=UMINUS_OP))
				{
					error("left side of assignment is not an l-value");
					pop_flag=FALSE;
				}
				else	
				if((expr->u.bin_op.arg1->tag==INT_CONST_NODE||expr->u.bin_op.arg1->tag==DOUBLE_CONST_NODE)&&
					(expr->u.bin_op.arg2->tag==INT_CONST_NODE||expr->u.bin_op.arg2->tag==DOUBLE_CONST_NODE))
				{
				
					EXPR temp=constant_fold(expr);
					eval(temp);		
				}
			else
			{
				
				if((expr->u.bin_op.op==EQUAL_OP)&&(expr->u.bin_op.arg2->tag==BINOP_NODE))
					{
					
						EXPR temp=expr->u.bin_op.arg2;
						if((temp->u.bin_op.arg1->tag==INT_CONST_NODE||temp->u.bin_op.arg1->tag==DOUBLE_CONST_NODE)&&
							(temp->u.bin_op.arg2->tag==INT_CONST_NODE||temp->u.bin_op.arg2->tag==DOUBLE_CONST_NODE))
						{
							EXPR temp1=constant_fold(temp);	
							
							temp1->type_of_node=expr->u.bin_op.arg1->type_of_node;
							eval(expr->u.bin_op.arg1);
							expr->u.bin_op.arg2=to_convert_rhs(expr->u.bin_op.arg1,temp1);
							eval(expr->u.bin_op.arg2);	
						}
						else
						{
							eval(expr->u.bin_op.arg1);				
							eval(expr->u.bin_op.arg2);	
						}
							
					}
				else{
				
				eval(expr->u.bin_op.arg1);
								//RECURSIVE CALLING
								
				if(expr->u.bin_op.arg2->tag == UNOP_NODE)
				{
					if(typenum(expr->u.bin_op.arg1->type_of_node)<typenum(expr->u.bin_op.arg2->type_of_node))
					{
					//	b_convert(expr->u.bin_op.arg1->type_of_node, expr->u.bin_op.arg2->type_of_node);
					//	expr->u.bin_op.arg1->type_of_node = expr->u.bin_op.arg1->type_of_node;
					
					//printf("inside if for UNOP_NODE arg1 = %d, arg2 = %d\n",expr->u.bin_op.arg1->type_of_node,expr->u.bin_op.arg2->type_of_node);
					convert(expr->u.bin_op.arg1->type_of_node,expr->u.bin_op.arg2->type_of_node, expr);
				//	expr->type_of_node = expr->u.bin_op.arg2->type_of_node;
				}
				}	
				
				eval(expr->u.bin_op.arg2);
			}
				switch(expr->u.bin_op.op)
				{
				
					/***********************ARITHMETIC OPERATORS*********************/
					case ADD_OP:
						{
							
							convert(expr->u.bin_op.arg1->type_of_node,expr->u.bin_op.arg2->type_of_node, expr);
							b_arith_rel_op(B_ADD,expr->type_of_node );
							
							break;
						}
					case SUB_OP:
						{
							convert(expr->u.bin_op.arg1->type_of_node,expr->u.bin_op.arg2->type_of_node, expr);
							b_arith_rel_op(B_SUB, expr->type_of_node);
							
							break;
						}
					case MULT_OP:
						{
							//b_arith_rel_op(B_MULT, get_cast_typetag(expr->u.bin_op.arg1->u.deref.deref_tag,expr->u.bin_op.arg2->u.deref.deref_tag));
							convert(expr->u.bin_op.arg1->type_of_node,expr->u.bin_op.arg2->type_of_node, expr);
							b_arith_rel_op(B_MULT,expr->type_of_node);
						
							break;
						}
					case DIV_OP:
						{
							convert(expr->u.bin_op.arg1->type_of_node,expr->u.bin_op.arg2->type_of_node, expr);
							b_arith_rel_op(B_DIV, expr->type_of_node);//get_cast_typetag(expr->u.bin_op.arg1->u.deref.deref_tag,expr->u.bin_op.arg2->u.deref.deref_tag));
							
							break;
						}
						
					/********COMPARISON OPERATOR******/	
					case NOT_EQUAL_OP:
						{
							//b_arith_rel_op(B_NE, get_cast_typetag(expr->u.bin_op.arg1->u.deref.deref_tag,expr->u.bin_op.arg2->u.deref.deref_tag));
							convert(expr->u.bin_op.arg1->type_of_node,expr->u.bin_op.arg2->type_of_node, expr);
							b_arith_rel_op(B_NE,expr->type_of_node);
							expr->type_of_node = TYSIGNEDINT;
							break;
						}
					case EQUAL_TO_OP:
						{
							convert(expr->u.bin_op.arg1->type_of_node,expr->u.bin_op.arg2->type_of_node, expr);
							b_arith_rel_op(B_EQ, expr->type_of_node);//get_cast_typetag(expr->u.bin_op.arg1->u.deref.deref_tag,expr->u.bin_op.arg2->u.deref.deref_tag));
							expr->type_of_node = TYSIGNEDINT;
							break;
						}
					case GREATER_THAN_EQUAL_OP:
						{
							convert(expr->u.bin_op.arg1->type_of_node,expr->u.bin_op.arg2->type_of_node, expr);
							b_arith_rel_op(B_GE, expr->type_of_node);//get_cast_typetag(expr->u.bin_op.arg1->u.deref.deref_tag,expr->u.bin_op.arg2->u.deref.deref_tag));
							expr->type_of_node = TYSIGNEDINT;
							break;
						}
					case LESS_THAN_EQUAL_OP:
						{
							convert(expr->u.bin_op.arg1->type_of_node,expr->u.bin_op.arg2->type_of_node, expr);
							b_arith_rel_op(B_LE, expr->type_of_node);//get_cast_typetag(expr->u.bin_op.arg1->u.deref.deref_tag,expr->u.bin_op.arg2->u.deref.deref_tag));
							expr->type_of_node = TYSIGNEDINT;
							break;
						}
					case GREATER_THAN_OP:
						{
							convert(expr->u.bin_op.arg1->type_of_node,expr->u.bin_op.arg2->type_of_node, expr);
							b_arith_rel_op(B_GT, expr->type_of_node);//get_cast_typetag(expr->u.bin_op.arg1->u.deref.deref_tag,expr->u.bin_op.arg2->u.deref.deref_tag));
							expr->type_of_node = TYSIGNEDINT;
							break;
						}
					case LESS_THAN_OP:
						{
							convert(expr->u.bin_op.arg1->type_of_node,expr->u.bin_op.arg2->type_of_node, expr);
							b_arith_rel_op(B_LT, expr->type_of_node);//get_cast_typetag(expr->u.bin_op.arg1->u.deref.deref_tag,expr->u.bin_op.arg2->u.deref.deref_tag));
							expr->type_of_node = TYSIGNEDINT;
							break;
						}	
					/******* ASSIGNMENT OPERATOR *******/
					case EQUAL_OP:
						{
							direct_assignment = FALSE; //Resetting here, Global flag, read comments at the top for its description
							
							ST_ID st_id = st_lookup_id(expr->u.bin_op.arg1->u.id);
							int * block_num;
							ST_DR temp = st_lookup(st_id, &block_num);
							TYPETAG tag = ty_query(temp->u.decl.type);
							//printf("tag is %d \n",tag);
							///**********************************************************NEW changes in if else******************************************/
							if(tag == TYFUNC || tag == TYARRAY)
								{
									error("left side of assignment is not an l-value");
								}
							else
							{
							
								if((expr->u.bin_op.arg2->type_of_node)!=(expr->u.bin_op.arg1->type_of_node))
								{
									b_convert(expr->u.bin_op.arg2->type_of_node,expr->u.bin_op.arg1->type_of_node);	
									expr->type_of_node = expr->u.bin_op.arg1->type_of_node;													
									}	
							//	printf("arg1 ID = %s...arg1 tag = %d....arg2 tag = %d \n",st_get_id_str(st_id),expr->u.bin_op.arg1->type_of_node,expr->u.bin_op.arg2->type_of_node);
								b_assign(tag);
								//b_pop();													
								pop_flag = TRUE;
							}
							break;
						}
				}
			}
				break;
			}				
	}
	return pop_flag;
}
TYPETAG get_cast_typetag(TYPETAG tag1, TYPETAG tag2)
{	
	
	if(tag1 == TYDOUBLE || tag2 == TYDOUBLE)
		{ return TYDOUBLE; }
	else /*if(tag1 == TYSIGNEDINT && tag2 == TYSIGNEDINT)*/
		{ return TYSIGNEDINT; }
		
}



void install_function(ST_ID st_id, TYPE f_type, BUCKET_PTR bucket, tr tree, PARAM_LIST param_list)
{	
	//TYPETAG temp = ty_query(build_id_param(build_base(bucket),tree));
	//int * block_num;
	//ST_DR temp = st_lookup(st_id, &block_num);
	//printf("temp is %d\n",temp);
	
	int * block_num;
	ST_DR st_dr = st_lookup(st_id, &block_num);	//Check to see if function is already installed
		
	PARAMSTYLE *paramstyle;
	PARAM_LIST *params;
	
		
		if(st_dr == NULL) //Function has no prototype declaration and this is the first time it is appearing, hence install it as FDECL
		{
			st_exit_block();
			//printf("stdr block is %d\n",st_get_cur_block());
			st_dr = stdr_alloc(); 
			st_dr->tag = FDECL; 
			st_dr->u.decl.type = build_id(build_base(bucket), tree);;
			BOOLEAN flag = st_install(st_id, st_dr);
			st_enter_block();	
			b_func_prologue(st_get_id_str(st_id));		//Emitting function name as label in assembly code, to be used as pointer to location later.	
			call_b_store_formal_param(param_list, st_id);		//local function to call b_store_formal_param for each parameter
			
		}
		
		else if(ty_query(st_dr->u.decl.type) !=TYFUNC)				//check if previous declaration was done as a function
		{
			error("duplicate or incompatible function declaration `%s\'",st_get_id_str(st_id));
		}
		
		else if(st_dr != NULL && st_dr->tag == FDECL)	//If it is already installed as FDECL then ERROR
		{
			error("duplicate or incompatible function declaration `%s\'",st_get_id_str(st_id));
		}
		
		else if(st_dr != NULL && st_dr->tag == GDECL  && ty_query_func(st_dr->u.decl.type,&paramstyle,&params) == f_type)	//If it is already installed as GDECL and 
		{																			//it has same return not as it was installed then change GDECL to FDECL
			//printf("ty_query(st_dr->u.decl.type) = %d \n",ty_query(st_dr->u.decl.type));
			st_dr->tag = FDECL;
		//	st_enter_block();
			b_func_prologue(st_get_id_str(st_id));		//Emitting function name as label in assembly code, to be used as pointer to location later.
			call_b_store_formal_param(param_list, st_id);		//local function to call b_store_formal_param for each parameter
		}
		else
		{
			//printf("func_query= %d....f_type = %d\n",ty_query(ty_query_func(st_dr->u.decl.type,&paramstyle,&params)), ty_query(f_type));
			error("Function has different return type");
			return;
		}
		//st_enter_block();
		
		/* call b_store_formal_param(TYPETAG) for each parameter in order from left to right*/
	}
	



/*******convertion*******/

void  convert(TYPETAG t1,TYPETAG t2, EXPR expr)
{
	//printf("Inside Convert\n t1 = %d..t2 = %d\n",t1,t2);
	
	if(typenum(t1) ==1 || typenum(t2) == 1  || typenum(t1) == 3 || typenum(t2) == 1)
	{
		//do nothing, This part of code must be unreachable since we are already converting char into int and float into double always as soon as they are entered
	//	printf("UNREACHABLE CODE REACHED..! \n");
	}
	else {	
	//		printf("inside first else in convert()\n");
		  if(typenum(t1)<typenum(t2))
		  {//	printf("inside second if in convert()\n");
		  	b_convert (t1, t2);
		  	expr->type_of_node = t2;
		  	expr->u.bin_op.arg1->type_of_node = t2;
		  }
		  else if(typenum(t1)>typenum(t2))
		  { //printf("inside second else if in convert()\n");
		  	b_convert (t2, t1);
		  	expr->type_of_node = t1;
		  	expr->u.bin_op.arg2->type_of_node = t1;
		  }
		}
}
 	


int typenum(TYPETAG t)
{
	
	if(t==TYDOUBLE)
	{
		return 4;
	}
	else if(t==TYFLOAT)
	{
		return 3;
	}
	else if(t== TYSIGNEDINT)
	{
		return 2;
		
	}
	else
	{
		return 1;
	}       
}

/************************************************************CONSTANT FOLDING FUNCTIONS*********************************************/
EXPR constant_fold(EXPR expr)
{
	EXPR temp1=expr->u.bin_op.arg1;
	EXPR temp2=expr->u.bin_op.arg2;
	EXPR ret;
	if(temp1->type_of_node==TYDOUBLE&&temp2->type_of_node==TYDOUBLE)
	{
		
		double i=temp1->u.value.double_const_value;
		double j=temp2->u.value.double_const_value;
		double val=const_switch(i,j,expr->u.bin_op.op);
		
		ret=make_double_const_node(val);	
	}
	else
	if(temp1->type_of_node==TYDOUBLE&&temp2->type_of_node==TYSIGNEDINT)
	{
		
		double i=temp1->u.value.double_const_value;
		double j=temp2->u.value.int_const_value;
		double val=const_switch(i,j,expr->u.bin_op.op);
		
		ret=make_double_const_node(val);	
	}
	else
	if(temp1->type_of_node==TYSIGNEDINT&&temp2->type_of_node==TYDOUBLE)
	{
		double i=temp1->u.value.int_const_value;
		double j=temp2->u.value.double_const_value;
		double val=const_switch(i,j,expr->u.bin_op.op);
		
		ret=make_double_const_node(val);
	}
	else
	{
		double i=temp1->u.value.int_const_value;
		double j=temp2->u.value.int_const_value;
		int val=(int)const_switch(i,j,expr->u.bin_op.op);
		ret=make_int_const_node(val);
	}
	
	return ret;	
}

double const_switch(double i,double j,BIN_OP_TYPE t)
{
	double ret;
	switch(t)
	{
		case ADD_OP:
			ret=i+j;
			break;
		case SUB_OP:
		 	ret =i-j;
			break;
		case MULT_OP:
			ret=i*j;
			break;
		case DIV_OP:
			ret=i/j;
			break;
		
	}
	return ret;
}


EXPR to_convert_rhs(EXPR lhs,EXPR rhs)
{
	EXPR ret;
	if(lhs->type_of_node==TYSIGNEDINT)
	{
		if(rhs->tag==DOUBLE_CONST_NODE)
		{
			int val=(int)rhs->u.value.double_const_value;
			ret=make_int_const_node(val);
		}
		else
		ret=rhs;
	}
	else
	{
		if(rhs->tag==INT_CONST_NODE)
		{
			double val=(double)rhs->u.value.int_const_value;
			ret=make_double_const_node(val);
		}
		else
		ret=rhs;
		
	}
	
	return ret;
}

/************************************************ For 90 Percent *********************************************************/

ARG_EXPR create_argument_expr_list(EXPR expr, ARG_EXPR arg_expr)
{
	//Insert at End
	ARG_EXPR new_arg_expr = (ARG_EXPR)malloc(sizeof(ARG_EXPR_NODE));
	new_arg_expr->expr_node = expr;
	
	if(arg_expr == NULL)
	{
		//create new argument expr list, if it the first node
		new_arg_expr->next = NULL;
		new_arg_expr->prev = NULL;
		arg_expr = new_arg_expr;
	}
	else
	{
		new_arg_expr->next = NULL;
		new_arg_expr->prev = arg_expr;
		arg_expr->next = new_arg_expr;
		arg_expr = new_arg_expr;
	}
	
	return arg_expr;
	
}

void print_argument_expr_list(ARG_EXPR arg_expr)
{	
	
	ARG_EXPR temp = arg_expr;
	if(temp == NULL)
	{
	//	printf("list is NULL \n");
	}
	else {
	//printf("PRINT - argument expr list is pointing to typetag %d\n", (temp->expr_node->type_of_node));
	while(temp->next != NULL)
	{
		temp = temp->next;
	//	printf("PRINT - argument expr list is pointing to typetag %d\n", (temp->expr_node->type_of_node));
	}
	}
}

ARG_EXPR reverse_arg_expr_list(ARG_EXPR arg_expr)
{
	ARG_EXPR temp = arg_expr;
	while(temp->prev != NULL)
	{
		temp = temp->prev;
	
	}
	return temp;
}

int get_num_arg_expr_list(ARG_EXPR arg_expr)
{
	int num = 1;
	ARG_EXPR temp = arg_expr;
	if(temp == NULL)
	{
		num = 0;//printf("list is NULL \n");			//MUST be Unreachable
	}
	else
	{	//printf("get_num_arg_expr_list - else\n");
		while(temp->next != NULL)
		{
		temp = temp->next;
		num = num + 1;	
		}
	}
	
	//printf("Returning number %d \n",num);
	return num;
}

void func_b_load_arg(ARG_EXPR arg_expr)
{
	BOOLEAN b;
	ARG_EXPR temp = arg_expr;
	if(temp == NULL)
	{
		//printf("list is NULL \n");			//MUST be Unreachable
	}
	else {
		//printf("PRINT - argument expr list is pointing to typetag %d\n", (temp->expr_node->type_of_node));
		b = eval(temp->expr_node);
		b_load_arg(temp->expr_node->type_of_node);	//call for first value in the list
	while(temp->next != NULL)
	{
		temp = temp->next;
		//printf("PRINT - argument expr list is pointing to typetag %d\n", (temp->expr_node->type_of_node));
		b = eval(temp->expr_node);
		b_load_arg(temp->expr_node->type_of_node);
	}
	}
}

void call_b_store_formal_param(PARAM_LIST param_list, ST_ID st_id)
{
	PARAM_LIST temp = param_list;
	ST_DR st_dr;
	int offset;
	if(temp == NULL)
	{
		//printf("NULL parameter list\n");
	}
	else 
	{
		//first value in the list
		if(temp->is_ref == TRUE)
				offset = b_store_formal_param(TYPTR);					//if it is a reference parametere
			else
				offset = b_store_formal_param(ty_query(temp->type));
		
		//install as PDECL in symbol table and set binding field of st_dr with the offset
		st_dr = stdr_alloc(); 
		st_dr->tag = PDECL; 
		st_dr->u.decl.type = temp->type;
		st_dr->u.decl.is_ref = temp->is_ref;
		TYPETAG t = ty_query(st_dr->u.decl.type);
		st_dr->u.decl.sc = NO_SC;
		st_dr->u.decl.binding = offset;
		BOOLEAN flag = st_install(temp->id, st_dr);	
		//printf("Id installed is %s..with tag = %d..and binding = %d\n",st_get_id_str(temp->id),st_dr->tag, st_dr->u.decl.binding);
		
													
		while(temp->next != NULL)
		{	
			//next values in the list
			temp = temp->next;
			
			if(temp->is_ref == TRUE)
				offset = b_store_formal_param(TYPTR);						//if it is a reference parametere
			else
				offset = b_store_formal_param(ty_query(temp->type));	
			
			//install as PDECL in symbol table and set binding field of st_dr with the offset
			st_dr = stdr_alloc(); 
			st_dr->tag = PDECL; 
			st_dr->u.decl.type = temp->type;
			st_dr->u.decl.is_ref = temp->is_ref;
			TYPETAG t = ty_query(st_dr->u.decl.type);
			st_dr->u.decl.sc = NO_SC;
			st_dr->u.decl.binding = offset;
			BOOLEAN flag = st_install(temp->id, st_dr);	
			//printf("Id installed is %s\n",st_get_id_str(temp->id));
			
		}
	}
}









