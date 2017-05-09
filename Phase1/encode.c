
/****************************************************************/
/*								*/
/*	CSCE531 - "Pascal" and "C" Compilers			*/
/*								*/
/*	--encode.c--						*/
/*								*/
/*	This File Contains routines that support the		*/
/*	"Message Module".					*/
/*								*/
/*								*/
/*								*/
/****************************************************************/

/*
void b_alloc_char (int init);
void b_alloc_short (int init);
void b_alloc_int (int init);
void b_alloc_long (long init);
void b_alloc_ptr (char *init);
void b_alloc_float (double init);
void b_alloc_double (double init);

void b_global_decl (char *id, int alignment, unsigned int size);

typedef enum {
    
	
*/
#include "defs.h"
#include "bucket.h"
#include <stdio.h>
#include <stdlib.h>
#include "message.h"

#include "encode.h"
#include "symtab.h"
#include "types.h"
#include "backend-x86.h"

unsigned int size=1;
unsigned int allignment=0;
void encode(TYPE type, ST_ID st_id)
{	
	size =1;
	allignment=0;
	unsigned int s_size = get_size_for_encode(type); //s-size is size from ger_size
	int allignment = get_allignment_for_encode(type);
	
	//TYPETAG t = ty_query(type);
	//int allignment = get_size_basic(t);
	//printf("SIZE %d\n",size);
	b_global_decl (st_get_id_str(st_id), allignment, s_size);
	b_skip(size);
}



int get_size_for_encode(TYPE type)
{
	//printf("get_size_for_encode \n");
	TYPETAG t = ty_query(type);
	
	DIMFLAG *dimflag;
	unsigned int *dim;
	
	
	//printf("TYPETAG = %d\n",t);
	
	if( t == TYUNSIGNEDCHAR || t == TYSIGNEDCHAR)
	{
		size = 1 * size;
	//	printf("size char = %d\n",size);
	}
	else if(t == TYSIGNEDINT || t == TYUNSIGNEDINT || t == TYFLOAT || t == TYPTR || t == TYUNSIGNEDLONGINT)
	{
		size = 4 * size;
	//	printf("size int = %d\n",size);
	}
	else if(t == TYDOUBLE || t == TYLONGDOUBLE || t == TYSIGNEDLONGINT)
	{
		size = 8 * size;
	//	printf("size double = %d\n",size);
	}
	else if(t == TYUNSIGNEDSHORTINT || t == TYSIGNEDSHORTINT)
	{
		size = 2 * size;
	//	printf("size shortint = %d\n",size);
	}
	else if(t == TYARRAY)
	{	
	//	printf("Inside TARRAY \n");
		TYPE temp = ty_query_array(type, &dimflag, &dim);
		int d = dim;
		
		//printf("size of dimension from size of is %d\n",s);
		if(d == 0)
		{
			error("illegal array dimension");
			
		}
				
	//	printf("Dimflag is %d\n",dimflag);
		if((&dimflag) == NO_DIM)
		{
			bug("illegal array dimension");
		}
		else
		{	
	//		printf("dim is %d\n",dim);
			
			
	//		printf("d= %d\n",d);
			
			size = (d) * size;
			//size = 5;
			
	//		printf("size array = %d\n",size);
			get_size_for_encode(temp);
		}
		
	}
	//printf("size before return = %d\n",size);
	return size;
}
	
int get_allignment_for_encode(TYPE type)
	{
		
	DIMFLAG *dimflag;
	unsigned int *dim;
	
//	printf("get_allignment_for_encode \n");
	TYPETAG t = ty_query(type);
	
	
	if( t == TYUNSIGNEDCHAR || t == TYSIGNEDCHAR)
	{
		allignment = 1;
	}
	else if(t == TYSIGNEDINT || t == TYUNSIGNEDINT || t == TYFLOAT || t == TYPTR || t == TYUNSIGNEDLONGINT)
	{
		allignment = 4;
	}
	else if(t == TYDOUBLE || t == TYLONGDOUBLE || t == TYSIGNEDLONGINT)
	{
		allignment = 8;
	}
	else if(t == TYUNSIGNEDSHORTINT || t == TYSIGNEDSHORTINT)
	{
		allignment = 2;
	}
	else if(t == TYARRAY)
	{
		TYPE temp = ty_query_array(type, &dimflag, &dim);
		
//		printf("Dimflag is %d\n",dimflag);
		if((&dimflag) == NO_DIM)
		{
			bug("invalid array size found");
		}
		else
		{	
		get_allignment_for_encode(temp);
		}
	}
//	printf("allignment %d\n",allignment);
	return allignment;
}
