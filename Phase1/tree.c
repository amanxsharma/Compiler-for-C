/*								*/
/*								*/
/*	CSCE 531 - "C++" compiler				*/
/*								*/
/*								*/
/*		Module Implementing "C/C++" Types		*/
/*								*/
/*	--tree.c--						*/
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

/* actual definition of tree */

PARAM_LIST this_param_list = NULL;
int function_is_parameter = 0;

//param_list in my global variable is getting overwritten by parameter list of my parameters



PARAM_LIST create_param_list(BUCKET_PTR bucket, tr tree, ST_ID st_id, PARAM_LIST param_list, BOOLEAN is_ref)
{
//	printf("Printing list inside create param \n");

	//param_list = plist_alloc();
	
	function_is_parameter = 1; //set this flag to indicate that function is inside parameter list anf for this ty_build_func() should be passes as NULL in third argument.
	
	int dup_flag=0; //Flag to indicate duplicates.
	
	PARAM_LIST temp=param_list;
	
	//	print_param_list(temp);
	
	while(temp!=NULL)
	{	
	
		//printf("temp_id = %s ... current id = %s \n", st_get_id_str(temp->id),st_get_id_str(st_id));
		if( strcmp(st_get_id_str(temp->id),st_get_id_str(st_id))==0)
		{
			//printf("inside strcmp\n");
			dup_flag=1;
			break;
		}
		temp=temp->prev;
	}
	
	if(dup_flag==0)
	{
	//	printf("Duplicate flag is 0 \n");
		PARAM_LIST new_param_list = plist_alloc();
		
		PARAM_LIST prev_node;
		PARAM_LIST next_node;
		
		TYPE type = build_id_param(build_base(bucket),tree);
		new_param_list->type = type;
		//printf("Tyoetag inside parameterlist for type is %d\n",ty_query(type));
		new_param_list->sc = NO_SC;
		new_param_list->qu = NO_QUAL;
		new_param_list->id = st_id;
		new_param_list->is_ref = is_ref;
		//printf("FLag for param list is set to %d\n", new_param_list->is_ref);
		if(param_list == NULL)
		{	
			/*old*/new_param_list->next = NULL;
			new_param_list->prev = NULL;
			//printf("param list is NULL\n");
			
			
			
			//new_param_list->prev = NULL;
			/*old*/param_list = new_param_list;
		//	printf("NULL - param_list is pointing to Id %s\n", st_get_id_str(param_list->id));
			
			
			
		}
		else
		{	
			/*old*///new_param_list->next = param_list;
			new_param_list->next= NULL;
			new_param_list->prev = param_list;
			param_list->next = new_param_list;
			/*old*/param_list=new_param_list;
		//	printf("\n\nNOT NULL - param_list is pointing to Id %s\n", st_get_id_str(param_list->id));
			prev_node = param_list->prev;
			//next_node = param_list->next;
		//	printf("NOT NULL - param_list-prev is pointing to Id %s\n", st_get_id_str(prev_node->id));
			//printf("NOT NULL - param_list-next is pointing to Id %s\n", st_get_id_str(next_node->id));
		}		
	}
	else
	{
		error("duplicate parameter declaration for %s",st_get_id_str(st_id));
	}
	
	//printf("Inside create param list function \n");	
	
	
	//ST_DR st_dr = stdr_alloc();
	
	//BOOLEAN flag = st_install(st_id, st_dr);
	//if(flag == FALSE){error("duplicate declaration for %s",st_get_id_str(st_id));}
	
	
	

	this_param_list = param_list;
//	print_param_list(this_param_list);
	return param_list;
}

/****************************************************************************************************/

PARAM_LIST reverse(PARAM_LIST r_param_list)
{
	PARAM_LIST tempr = r_param_list;
	while(tempr->prev != NULL)
	{
		tempr = tempr->prev;
	//	printf("REVERSE - param_list is pointing to Id %s\n", st_get_id_str(tempr->id));
	}
	this_param_list = tempr;
	return tempr;
}

/****************************************************************************************************/

void print_param_list(PARAM_LIST r_param_list)
{	
	
	PARAM_LIST tempr = r_param_list;
	if(tempr == NULL)
	{
		printf("list is NULL \n");
	}
	else {
	printf("PRINT - param_list is pointing to Id %s\n", st_get_id_str(tempr->id));
	while(tempr->next != NULL)
	{
		tempr = tempr->next;
		printf("PRINT - param_list is pointing to Id %s\n", st_get_id_str(tempr->id));
	}
	}
}

/****************************************************************************************************/

tr push(char *n,char *d, tr next)
{
	tr node=(tr)malloc(sizeof(tree));
	assert(node);
	
	node->name=strdup(n);
	node->data=strdup(d);
	node->nextNode=next;
	//printf("name : %s    data : %s\n",node->name,node->data);
//	printf("PUSHING - %s identifier %s \n", node->name, node->data);
return node;	
}


TYPE build_id_param(TYPE tag,struct IdentifierTree* Tree)
{
	//printf("Tree->name = %s, Tree->data = %s\n",Tree->name,Tree->data);
	TYPE var=tag;
	tr new_node=Tree;
	
	while(new_node!=NULL)
	{
		//printf("Node->name = %s, Node->data = %s\n",new_node->name,new_node->data);
		if(strcmp((new_node->name),"pointer")==0)
		{
		//	printf("building for %s\n", new_node->name);
			
			var=ty_build_ptr(var, NO_QUAL);
		}
		else
			if(strcmp((new_node->name),"array")==0)
			{
			//	printf("building for %s\n", new_node->name);
				TYPETAG func_error = ty_query(var);
				if(func_error == TYFUNC)
				{
					error("cannot have array of functions");
				}
				else{
				
				var=ty_build_array(var,DIM_PRESENT,atoi(new_node->data));
			}
			}
		else
			if(strcmp((new_node->name),"identifier")==0)
			{
		//		printf("building for %s\n", new_node->name);
				
			}
		else 
			if(strcmp((new_node->name),"reference")==0)
			{
		//		printf("building for %s\n", new_node->name);
				
				
			}
		else
			if(strcmp((new_node->name),"function")==0)
			{
				//struct param* parameter_list = NULL;
				
	//			printf("building for %s with data = %s\n", new_node->name,new_node->data);
				
				TYPETAG func_error = ty_query(var);
				if(func_error == TYARRAY)
				{
					
					error("cannot have function returning array");
				}
				else if(func_error == TYFUNC)
				{
					
					error("cannot have function returning function");
				}
				else{
				//	printf("Printing list inside building \n");
				//	print_param_list(this_param_list);
				
				if(function_is_parameter == 1)
				{
					var = ty_build_func(var, PROTOTYPE, NULL);
					function_is_parameter = 0;
				}
				else
				{
					var = ty_build_func(var, PROTOTYPE, this_param_list);
				}
			}
			this_param_list = NULL;
			}
		new_node=new_node->nextNode;
	}
	
	TYPETAG t = ty_query(var);
	//printf("Type tag in build id = %d \n",t);
	return var;
}

/*********************************************************************************************************/
TYPE build_id(TYPE tag,struct IdentifierTree* Tree)
{
	//printf("Tree->name = %s, Tree->data = %s\n",Tree->name,Tree->data);
	TYPE var=tag;
	tr new_node=Tree;
	
	while(new_node!=NULL)
	{
		//printf("Node->name = %s, Node->data = %s\n",new_node->name,new_node->data);
		if(strcmp((new_node->name),"pointer")==0)
		{
		//	printf("building for %s\n", new_node->name);
			
			var=ty_build_ptr(var, NO_QUAL);
		}
		else
			if(strcmp((new_node->name),"array")==0)
			{
			//	printf("building for %s\n", new_node->name);
				TYPETAG func_error = ty_query(var);
				if(func_error == TYFUNC)
				{
					error("cannot have array of functions");
				}
				else{
				
				var=ty_build_array(var,DIM_PRESENT,atoi(new_node->data));
			}
			}
		else
			if(strcmp((new_node->name),"identifier")==0)
			{
		//		printf("building for %s\n", new_node->name);
				
			}
		else 
			if(strcmp((new_node->name),"reference")==0)
			{
		//		printf("building for %s\n", new_node->name);
				
				
			}
		else
			if(strcmp((new_node->name),"function")==0)
			{
				//struct param* parameter_list = NULL;
				
		//		printf("building for %s with data = %s\n", new_node->name,new_node->data);
				
				TYPETAG func_error = ty_query(var);
				if(func_error == TYARRAY)
				{
					
					error("cannot have function returning array");
				}
				else if(func_error == TYFUNC)
				{
					
					error("cannot have function returning function");
				}
				else{
			//		printf("Printing list inside building \n");
			//		print_param_list(this_param_list);
				
				//if(function_is_parameter == 1)
				//{
				//	var = ty_build_func(var, PROTOTYPE, NULL);
				//	function_is_parameter = 0;
				//}
				//else
				//{
					var = ty_build_func(var, PROTOTYPE, this_param_list);
				//}
			}
			//this_param_list = NULL;
			}
		new_node=new_node->nextNode;
	}
	
	TYPETAG t = ty_query(var);
	//printf("Type tag in build id = %d \n",t);
	return var;
}

/************************************************************************************************************************/

int update_expr_int(int i)
{
	struct expression* ret_expr = (struct expression*)malloc(sizeof(struct expression));
	ret_expr->tag = "INTEGER";
	ret_expr->u.int_value = i;
	return ret_expr->u.int_value ;
}
double update_expr_double(double d)
{
	struct expression* ret_expr = (struct expression*)malloc(sizeof(struct expression));
	ret_expr->tag = "DOUBLE";
	ret_expr->u.double_value = d;
	return ret_expr->u.double_value;
}





