/****************************************************************************/
/* 
	definitions to support tree.c
*/

#include "bucket.h"
typedef struct IdentifierTree
	{
		char *name;
		char *data;
		struct IdentifierTree *nextNode;
	}tree,*tr;



tr push(char *n,char *d, tr next);
TYPE build_id(TYPE tag,struct IdentifierTree* Tree);
TYPE build_id_param(TYPE tag,struct IdentifierTree* Tree);
void nothing(struct IdentifierTree* t);

typedef struct expression
{
	char * tag;
	union value
	{
		long int int_value;
		double double_value;
	}u;
}expr, *EXPR;
int update_expr_int(int i);
double update_expr_double(double d);

PARAM_LIST create_param_list(BUCKET_PTR bucket, tr tree, ST_ID st_id, PARAM_LIST param_list, BOOLEAN is_ref);
PARAM_LIST reverse(PARAM_LIST param_list);
void print_param_list(PARAM_LIST r_param_list);
