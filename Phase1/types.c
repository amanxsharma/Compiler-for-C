/*								*/
/*								*/
/*	CSCE 531 - "C++" compiler				*/
/*								*/
/*								*/
/*		Module Implementing "C/C++" Types		*/
/*								*/
/*	--types.c--						*/
/*								*/
/*	This module contains the routines that manipulate 	*/
/*	the type structure for the CSCE 531 "C++" compiler.	*/
/* 	Types are implemented by structure sharing.  So		*/
/*	the same structure is shared by two types if and only	*/
/*	if the types are considered "equal". Unfortunately	*/
/*	structure sharing doesn't work for arrays without	*/
/*	dimensions so special care must be taken with these     */
/*	(and with any type in which one of these could be	*/
/*	nested).						*/
/*								*/
/*								*/
/*								*/
/****************************************************************/

#include <stdlib.h>
#include "defs.h"
#include "types.h"
#include "symtab.h"
#include "message.h"

/* actual definition of TYPE */

typedef struct type_rec
{
    TYPETAG tag;
    union
    {
        struct /* C/C++ only */
        {
            DIMFLAG dimflag;
            unsigned int dim;
            struct type_rec *object;
        } array;
	struct /* Pascal only */
	{
	    INDEX_LIST indices;
	    struct type_rec *object;
	} pascal_array;
	struct /* Pascal only */
	{
	    struct type_rec *object;
	} set;
        struct /* C/C++ only */
        {
            PARAMSTYLE paramstyle;
            PARAM_LIST params;
            struct type_rec *object;
        } function;
	struct /* Pascal only */
	{
	    PARAM_LIST params;
	    BOOLEAN check_args;
	    struct type_rec *object;
	} pascal_function;
        struct /* C/C++ only */
        {
            TYPE_QUALIFIER qual;
            struct type_rec *object;
        } pointer;
	struct /* Pascal only */
	{
	    struct type_rec *object;
	    ST_ID id;
	} pascal_pointer;
        struct
        {
            ST_ID tagname; /* C/C++ only */
            MEMBER_LIST members;
        } struct_union;
        struct /* C/C++ only */
        {
            ST_ID tagname;
        } enumeration;
	struct /* Pascal only */
	{
	    unsigned int num_values;
	} pascal_enumeration;
        struct /* C/C++ only */
        {
            unsigned int length;
            struct type_rec *object;
        } bitfield;
	struct /* Pascal only */
	{
	    struct type_rec *base_type;
	    long low, high;
	} subrange;
    } u;	
} TYPE_REC;

static void print_type_pascal();
static void print_type_c(TYPE typein, BOOLEAN recurse_flag);
static BOOLEAN test_equality();
static void add_storage_list();
static TYPE check_object_func();
static TYPE check_object_bitfield();
static TYPE check_object_arr();
static TYPE check_object_ptr();
static BOOLEAN check_params_equality();
static BOOLEAN check_same_params();

/* Declaration Of All Basic Type Records */

static TYPE_REC basic_error;	/* Added 11/13/91 -SF */
static TYPE_REC basic_void;
static TYPE_REC basic_float;
static TYPE_REC basic_double;
static TYPE_REC basic_long_double;
static TYPE_REC basic_signed_long_int;
static TYPE_REC basic_signed_short_int;
static TYPE_REC basic_signed_int;
static TYPE_REC basic_unsigned_long_int;
static TYPE_REC basic_unsigned_short_int;
static TYPE_REC basic_unsigned_int;
static TYPE_REC basic_unsigned_char;
static TYPE_REC basic_signed_char;


/****************************************************************/
/*								*/
/*								*/
/*		Type Routines For Building Types		*/
/*			(Exported Routines)			*/
/*								*/
/****************************************************************/

/***************************************************************/
void ty_types_init()
{
    basic_error.tag = TYERROR;	/* Added 11/13/91 -SF */
    basic_void.tag = TYVOID;
    basic_float.tag = TYFLOAT;
    basic_double.tag = TYDOUBLE;
    basic_long_double.tag = TYLONGDOUBLE;
    basic_signed_long_int.tag = TYSIGNEDLONGINT;
    basic_signed_short_int.tag = TYSIGNEDSHORTINT;
    basic_signed_int.tag = TYSIGNEDINT;
    basic_unsigned_long_int.tag = TYUNSIGNEDLONGINT;
    basic_unsigned_short_int.tag = TYUNSIGNEDSHORTINT;
    basic_unsigned_int.tag = TYUNSIGNEDINT;
    basic_unsigned_char.tag = TYUNSIGNEDCHAR;
    basic_signed_char.tag = TYSIGNEDCHAR;
}

/***************************************************************/
TYPETAG ty_query(TYPE type)
{
    if (type == NULL)
        fatal ("NULL type in ty_query (%d %s).", __LINE__, __FILE__);

    return type->tag;
}

/**************************************************************/
BOOLEAN ty_test_equality(TYPE type1, TYPE type2)
{
#ifdef PASCAL_LANG
    /*
      Check for compatible types.  This is a wrapper for test_equality(),
      which avoids recursing infinitely on cyclic type structures by
      limiting its pointer depth to 2.
    */
    return test_equality(type1, type2, 2);
#else
			/*
			   This is complicated because structure sharing
			   doesn't work perfectly. In particular arrays
			   without dimensions must be handled gingerly.
			   And since types are nested, nested type
			   must be handled gingerly (since there could
			   be an array without a dimension lurking
			   somewhere within).
			*/
    if (type1 == NULL || type2 == NULL)
    {
	bug("Null argument(s) to \"ty_test_equality\"");
	return FALSE;
    }

    if (type1 == type2)
        return(TRUE);
    if (type1->tag != type2->tag)
        return(FALSE);

    switch(type1->tag) {
        case TYARRAY:
                /* recursive call to check element type */
            if (!ty_test_equality(type1->u.array.object,
                                  type2->u.array.object))
                return FALSE;
            else
            {
                    /* 
                       array without dimension is equal to any
                       other array of same element type. but if
                       both arrays have dimensions, they must be
                       the same.
                    */
                if ((type1->u.array.dimflag ==
                     type2->u.array.dimflag) &&
                    (type1->u.array.dimflag == DIM_PRESENT) &&
                    (type1->u.array.dim != type2->u.array.dim))
                    return FALSE;
                else
                    return TRUE;
            }
        case TYFUNC:
                /*
                  recursive call to check return type.
                  paramstyles must also be the same.
                */
            if (!ty_test_equality(type1->u.function.object,
                                  type2->u.function.object) ||
                (type1->u.function.paramstyle !=
                 type2->u.function.paramstyle))
                return FALSE;
            else
            {
                    /*
                      if function prototype present (with or
                      without elipsis) check parameter lists
                    */

                if (type1->u.function.paramstyle != OLDSTYLE)
                    return check_params_equality(
                        type1->u.function.params,
                        type2->u.function.params);
                else
                    return TRUE;
            }
        case TYPTR:
                /*
                  recursive call to check type pointed to
                  (qualifiers must be the same)
                */
            if (type1->u.pointer.qual ==
                type2->u.pointer.qual)
                return ty_test_equality(type1->u.pointer.object,
                                        type2->u.pointer.object);
            else
                return FALSE;
        default:
                /* structure sharing works for all other types */
            return FALSE;
    }
#endif		
}
	    
/***************************************************************/
TYPE ty_strip_modifier(TYPE type)
{
    switch (type->tag)
    {
        case TYARRAY :
            return type->u.array.object;
            break;

        case TYFUNC :
            return type->u.function.object;
            break;

        case TYPTR :
            return type->u.pointer.object;
            break;

        case TYSET :
            return type->u.set.object;

        case TYSUBRANGE :
            return type->u.subrange.base_type;

        default:
            return (NULL);
    }
}

/***************************************************************/
TYPE ty_build_basic(TYPETAG tag)
{
    switch (tag)
    {
        case TYFLOAT :
            return &basic_float;

        case TYDOUBLE :
            return &basic_double;

        case TYLONGDOUBLE :
            return &basic_long_double;

        case TYUNSIGNEDINT :
            return &basic_unsigned_int;

        case TYUNSIGNEDCHAR :
            return &basic_unsigned_char;

        case TYUNSIGNEDSHORTINT :
            return &basic_unsigned_short_int;

        case TYUNSIGNEDLONGINT :
            return &basic_unsigned_long_int;

        case TYSIGNEDCHAR :
            return &basic_signed_char;

        case TYSIGNEDINT :
            return &basic_signed_int;

        case TYSIGNEDLONGINT:
            return &basic_signed_long_int;

        case TYSIGNEDSHORTINT:
            return &basic_signed_short_int;

        case TYVOID :
            return &basic_void;

        case TYERROR :		/* Added 11/13/91 -SF */
            return &basic_error;

        default: 
            bug("illegal tag in \"ty_build_basic\" \n");
    }
}


/****************************************************************/
TYPE ty_build_array_pascal(TYPE object, INDEX_LIST indices)
{
    TYPE ret_type;

    ret_type = (TYPE)malloc(sizeof(TYPE_REC));
    ret_type->tag = TYARRAY;
    ret_type->u.pascal_array.indices = indices;
    ret_type->u.pascal_array.object = object;
    return(ret_type);
}

/*****************************************************************/
TYPE ty_build_unresolved_ptr(ST_ID id)
{
    TYPE ret_type;

    if (id == NULL)
	fatal("NULL id passed to \"ty_build_unresolved_ptr\"");

    ret_type = (TYPE)malloc(sizeof(TYPE_REC));
    ret_type->tag = TYPTR;
    ret_type->u.pascal_pointer.object = NULL;
    ret_type->u.pascal_pointer.id = id;

    return(ret_type);
}

/*****************************************************************/
TYPE ty_build_ptr_pascal(TYPE object)
{
    TYPE ret_type;

    if (object == NULL)
	fatal("NULL object passed to \"ty_build_ptr\"");

    ret_type = (TYPE)malloc(sizeof(TYPE_REC));
    ret_type->tag = TYPTR;
    ret_type->u.pascal_pointer.object = object;
    ret_type->u.pascal_pointer.id = NULL;

    return(ret_type);
}

/*****************************************************************/
TYPE ty_build_ptr(TYPE object, TYPE_QUALIFIER qualifier)
{
    TYPE ret_type;

        /* check for equivalent type on list; if so reuse */
    if ((ret_type = check_object_ptr(object,qualifier)) == NULL)
    {

            /* allocate new structure */
        ret_type = (TYPE)malloc(sizeof(TYPE_REC));
        ret_type->tag = TYPTR;
        ret_type->u.pointer.qual = qualifier;
        ret_type->u.pointer.object = object;
        add_storage_list(ret_type,TYPTR);
    }

    return(ret_type);
}

/****************************************************************/
TYPE ty_build_array(TYPE object, DIMFLAG dimflag, int dimension)
{
    TYPE ret_type;

        /* check for equivalent type on list if so reuse */
    if ((ret_type = check_object_arr(object,dimflag,dimension)) == NULL)
    {

            /* allocate new structure */
        ret_type = (TYPE)malloc(sizeof(TYPE_REC));
        ret_type->tag = TYARRAY;
        ret_type->u.array.dimflag = dimflag;
        ret_type->u.array.dim = dimension;
        ret_type->u.array.object = object;
        add_storage_list(ret_type,TYARRAY);
    }
    return(ret_type);
}        

/****************************************************************/
TYPE ty_build_set(TYPE object)
{
    TYPE ret_type;

    ret_type = (TYPE)malloc(sizeof(TYPE_REC));
    ret_type->tag = TYSET;
    ret_type->u.set.object = object;
    return(ret_type);
}

/****************************************************************/
TYPE ty_build_struct(ST_ID tagname, MEMBER_LIST members)
{
    TYPE ret_type;

	/* allocate new structure */
    ret_type = (TYPE)malloc(sizeof(TYPE_REC));
    ret_type->tag = TYSTRUCT;
    ret_type->u.struct_union.tagname = tagname;
    ret_type->u.struct_union.members = members;
    return(ret_type);
}        

/****************************************************************/
TYPE ty_build_union(ST_ID tagname, MEMBER_LIST members)
{
    TYPE ret_type;

	/* allocate new structure */
    ret_type = (TYPE)malloc(sizeof(TYPE_REC));
    ret_type->tag = TYUNION;
    ret_type->u.struct_union.tagname = tagname;
    ret_type->u.struct_union.members = members;
    return(ret_type);
}        

/****************************************************************/
TYPE ty_build_enum_pascal(unsigned int num_values)
{
    TYPE ret_type;

    ret_type = (TYPE)malloc(sizeof(TYPE_REC));
    ret_type->tag = TYENUM;
    ret_type->u.pascal_enumeration.num_values = num_values;
    return(ret_type);
}

/****************************************************************/
TYPE ty_build_enum(ST_ID tagname)
{
    TYPE ret_type;

	/* allocate new structure */
    ret_type = (TYPE)malloc(sizeof(TYPE_REC));
    ret_type->tag = TYENUM;
    ret_type->u.enumeration.tagname = tagname;
    return(ret_type);
}


/**************************************************************/
TYPE ty_build_bitfield(TYPE object, unsigned int length)
{
    TYPE ret_type;
	    
        /* check for equivalent type on list; if so reuse */
    if ((ret_type = check_object_bitfield(object,length)) == NULL)
    {
            /* allocate new structure */
        ret_type = (TYPE)malloc(sizeof(TYPE_REC));
        ret_type->tag = TYBITFIELD;
        ret_type->u.bitfield.length = length;
        ret_type->u.bitfield.object = object;
        add_storage_list(ret_type,TYBITFIELD);
    }

    return(ret_type);
}        

/****************************************************************/
TYPE ty_build_func_pascal(TYPE object, PARAM_LIST params, BOOLEAN check_args)
{
    TYPE ret_type;

    ret_type = (TYPE)malloc(sizeof(TYPE_REC));
    ret_type->tag = TYFUNC;
    ret_type->u.pascal_function.params = params;
    ret_type->u.pascal_function.check_args = check_args;
    ret_type->u.pascal_function.object = object;
    return(ret_type);
}

/****************************************************************/
TYPE ty_build_func(TYPE object, PARAMSTYLE paramstyle, PARAM_LIST params)
{
    TYPE ret_type;

        /* check for equivalent type on list if so reuse */
    if ((ret_type = check_object_func(object,paramstyle,params))==NULL)
    {

            /* allocate new structure */
        ret_type = (TYPE)malloc(sizeof(TYPE_REC));
        ret_type->tag = TYFUNC;
        ret_type->u.function.paramstyle = paramstyle;
        ret_type->u.function.params = params;
        ret_type->u.function.object = object;
        add_storage_list(ret_type,TYFUNC);
    }

    return(ret_type);
}        

/****************************************************************/
TYPE ty_build_subrange(TYPE object, long low, long high)
{
    TYPE ret_type;

    ret_type = (TYPE)malloc(sizeof(TYPE_REC));
    ret_type->tag = TYSUBRANGE;
    ret_type->u.subrange.base_type = object;
    ret_type->u.subrange.low = low;
    ret_type->u.subrange.high = high;
    return(ret_type);
}

/****************************************************************/
TYPE ty_query_func_pascal(TYPE type, PARAM_LIST * params, BOOLEAN * check_args)
{
    if (type->tag != TYFUNC)
	bug("not passed a function in \"ty_query_func\"");
    *params = type->u.function.params;
    *check_args = type->u.pascal_function.check_args;
    return type->u.function.object;
}

/****************************************************************/
TYPE ty_query_func(TYPE type, PARAMSTYLE *paramstyle, PARAM_LIST *params)
{
    if (type->tag != TYFUNC)
        bug("not passed a function in \"ty_query_func\"");
    *paramstyle = type->u.function.paramstyle;
    *params = type->u.function.params;
    return type->u.function.object;
}


/****************************************************************/
MEMBER_LIST ty_retrieve_members(TYPE type)
{
    switch (type->tag)
    {
        case TYSTRUCT:
        case TYUNION:
            return type->u.struct_union.members;
        default:
            bug("illegal type in \"ty_retrieve_members\"");
    }
}

/****************************************************************/
ST_ID ty_retrieve_tagname(TYPE type)
{

    switch (type->tag)
    {
        case TYSTRUCT:
        case TYUNION:
            return type->u.struct_union.tagname;
        case TYENUM:
            return type->u.enumeration.tagname;
        default:
            bug("illegal type in \"ty_retrieve_tagname\"");
    }
}

/****************************************************************/
TYPE ty_query_array_pascal(TYPE type, INDEX_LIST * indices)
{
    if (type->tag != TYARRAY)
	bug("illegal type in \"ty_query_array\"");
    *indices = type->u.pascal_array.indices;
    return type->u.pascal_array.object;
}

/****************************************************************/
TYPE ty_query_array(TYPE type, DIMFLAG *dimflag, unsigned int *dim)
{

    if (type->tag != TYARRAY)
        bug("illegal type in \"ty_query_array\"");
    *dimflag = type->u.array.dimflag;
    *dim = type->u.array.dim;

    return type->u.array.object;
}

/****************************************************************/
TYPE ty_query_bitfield(TYPE type, unsigned int *length)
{
    if (type->tag != TYBITFIELD)
        bug("illegal type in \"ty_query_bitfield\"");
    *length = type->u.bitfield.length;
    return type->u.bitfield.object;
}

/****************************************************************/
TYPE ty_query_set(TYPE type)
{
    if (type->tag != TYSET)
	bug("illegal type in \"ty_query_set\"");
    return type->u.set.object;
}

/****************************************************************/
TYPE ty_query_ptr_pascal(TYPE type, ST_ID * id)
{
    if (type->tag != TYPTR)
	bug("illegal type in \"ty_query_ptr\"");

    *id = type->u.pascal_pointer.id;

    return type->u.pascal_pointer.object;
}

/****************************************************************/
TYPE ty_query_pointer(TYPE type, TYPE_QUALIFIER *qual)
{
    if (type->tag != TYPTR)
        bug("illegal type in \"ty_query_pointer\"");
    *qual = type->u.pointer.qual;
    return type->u.pointer.object;
}

/****************************************************************/
TYPE ty_query_subrange(TYPE type, long * low, long * high)
{
    if (type->tag != TYSUBRANGE)
	bug("illegal type in \"ty_query_subrange\"");
    *low = type->u.subrange.low;
    *high = type->u.subrange.high;
    return type->u.subrange.base_type;
}

/****************************************************************/
unsigned int ty_query_enum(TYPE type)
{
    if (type->tag != TYENUM)
	bug("Illegal type in \"ty_query_enum\"");
    return type->u.pascal_enumeration.num_values;
}

 
/****************************************************************/
TYPE ty_assign_members(TYPE type, MEMBER_LIST members)
{
    switch (type->tag)
    {
        case TYSTRUCT:
        case TYUNION:
            type->u.struct_union.members = members;
            break;
        default:
            bug("illegal tag in \"ty_assign_members\" ");
            break;
    }
    return(type);
}

/****************************************************************/
BOOLEAN ty_resolve_ptr(TYPE ptr_type, TYPE object_type)
{
    ST_ID id;

    if (ptr_type == NULL || object_type == NULL)
    {
	msg("null argument(s) to \"ty_resolve_ptr\"");
	return FALSE;
    }

    if (ty_query(ptr_type) != TYPTR)
    {
	msg("Nonpointer type in \"ty_resolve_ptr\"");
	return FALSE;
    }

    if (ty_query_ptr_pascal(ptr_type,&id) != NULL)
    {
	msg("Object type already resolved in \"ty_resolve_ptr\"");
	return FALSE;
    }

        /* Modify the object type of the pointer */
    ptr_type->u.pascal_pointer.object = object_type;
    ptr_type->u.pascal_pointer.id = NULL;

    return TRUE;
}

/*****************************************************************/
/*
	designed to delete a TYPE: does nothing
*/
void ty_delete(TYPE type)
{
        /* not implemented */
}


/********************************************************************/
/*
  TYPE printing routines

  All these routines use "msg/msgn" from message.c for printing.
*/
/********************************************************************/
/*
  The problem with these routines is to avoid infinite loops with cyclical
  (recursive) types. Such types are handled differently in Pascal versus
  C/C++. In C/C++, struct/union tags can be used before the corresponding
  struct/union is declared, but pointer types can only be declared after
  the object type is declared. In Pascal, by contrast, pointer types can
  refer to as yet undeclared object types, and so, unlike C/C++, cycles in
  Pascal types need not involve records.

  In C/C++, cycles can only happen with struct/union. In this case, there
  is an inner routine that takes a "recurse flag" and actually does the work
  of displaying the type. The flag will be passed in as TRUE when
  ty_print_type is called normally. But will be passed in as FALSE when
  called from ty_print_memlist. When called with FALSE, the routine will not
  attempt to print a member list.

  For Pascal, we check for cycles not going through records or
  procedure/function parameter lists or array index lists (cycles in
  array index types would be illegal anyway) by running two pointers
  through the type at different speeds (the so-called tortoise & hare
  method).  The slow pointer moves at half speed by our toggling
  cycle_check each time we call print_type directly.
  Simultaneously, we will use the recurse flag to only go through
  one level of records/procedures/functions.
*/
void ty_print_type_pascal(TYPE typein)
{
    print_type_pascal(typein,TRUE,typein,FALSE);
}


void ty_print_type(TYPE typein)
{
    print_type_c(typein, TRUE);
}


static void print_type_c(TYPE typein, BOOLEAN recurse_flag)
{
    unsigned int dim;
    DIMFLAG dimflag;
    unsigned int length;
    PARAMSTYLE paramstyle;
    PARAM_LIST params;
    TYPE_QUALIFIER qualifier;
    TYPE object;
    TYPETAG tag;

    if (typein == NULL)
    {
        msgn("<null>");
        return;
    }

    tag = typein->tag;

    switch (tag)
    {
        case TYARRAY:
            object = ty_query_array(typein,&dimflag,&dim);
            if (dimflag == DIM_PRESENT)
                msgn("array of %d ",dim);
            else
                msgn("array of ");
            print_type_c(object,recurse_flag);
            break;

        case TYBITFIELD:
            object = ty_query_bitfield(typein,&length);
            print_type_c(object,recurse_flag);
            msgn(" bitfield : %d",length);
            break;

        case TYPTR:
            object = ty_query_pointer(typein,&qualifier);
            ty_print_qualifier(qualifier);
            msgn(" pointer to ");
            print_type_c(object,recurse_flag);
            break;

        case TYSTRUCT:
            msg("structure %s",
                st_get_id_str(typein->u.struct_union.tagname));
            if (recurse_flag)
                ty_print_memlist(typein->u.struct_union.members);
            break;

        case TYUNION:
            msg("union %s",
                st_get_id_str(typein->u.struct_union.tagname));
            if (recurse_flag)
                ty_print_memlist(typein->u.struct_union.members);
            break;

        case TYENUM:
            msgn("enumeration %s",
                 st_get_id_str(typein->u.enumeration.tagname));
            break;

        case TYFUNC:
            object = ty_query_func(typein,&paramstyle,&params);
            if (paramstyle == OLDSTYLE)
            {
                msgn("function returning ");
                print_type_c(object,recurse_flag);
            }
            else
            {
                msgn("function prototype");
                if (paramstyle == DOT_DOT_DOT)
                    msgn(" (elipsis)");
                ty_print_paramlist(params);
                msgn("\t\treturning ");
                print_type_c(object,recurse_flag);
            }
            break;

        case TYFLOAT:
            msgn("float");
            break;

        case TYDOUBLE:
            msgn("double");
            break;

        case TYLONGDOUBLE:
            msgn("long double");
            break;

        case TYUNSIGNEDINT:
            msgn("unsigned int");
            break;

        case TYUNSIGNEDCHAR:
            msgn("unsigned char");
            break;

        case TYUNSIGNEDSHORTINT:
            msgn("unsigned short int");
            break;

        case TYUNSIGNEDLONGINT:
            msgn("unsigned long int");
            break;

        case TYSIGNEDCHAR:
            msgn("signed char");
            break;

        case TYSIGNEDINT:
            msgn("signed int");
            break;

        case TYSIGNEDLONGINT:
            msgn("signed long int");
            break;

        case TYSIGNEDSHORTINT:
            msgn("signed short int");
            break;

        case TYVOID:
            msgn("void");
            break;

        case TYERROR:
            msgn("error");
            break;

        default:
            bug("illegal typetag (%d) in \"ty_print_type\"", tag);
    }

}

/********************************************************************/
void ty_print_typetag(TYPETAG tag)
{

    switch (tag)
    {
        case TYARRAY:
            msgn("array");
            break;

        case TYSET:
            msgn("set");
            break;

        case TYBITFIELD:
            msgn("bitfield");
            break;

        case TYPTR:
            msgn("pointer to");
            break;

        case TYSTRUCT:
            msgn("structure");
            break;

        case TYUNION:
            msgn("union");
            break;

        case TYENUM:
            msgn("enumeration");
            break;

        case TYFUNC:
            msgn("function returning");
            break;

        case TYSUBRANGE:
            msgn("subrange");
            break;

        case TYFLOAT:
            msgn("float");
            break;

        case TYDOUBLE:
            msgn("double");
            break;

        case TYLONGDOUBLE:
            msgn("long double");
            break;

        case TYUNSIGNEDINT:
            msgn("unsigned int");
            break;

        case TYUNSIGNEDCHAR:
            msgn("unsigned char");
            break;

        case TYUNSIGNEDSHORTINT:
            msgn("unsigned short int");
            break;

        case TYUNSIGNEDLONGINT:
            msgn("unsigned long int");
            break;

        case TYSIGNEDCHAR:
            msgn("signed char");
            break;

        case TYSIGNEDINT:
            msgn("signed int");
            break;

        case TYSIGNEDLONGINT:
            msgn("signed long int");
            break;

        case TYSIGNEDSHORTINT:
            msgn("signed short int");
            break;

        case TYVOID:
            msgn("void");
            break;

        case TYERROR:
            msgn("error");
            break;

        default:
            bug("illegal tag in \"ty_print_typetag\"");
    }

}

/****************************************************************/
void ty_print_qualifier(TYPE_QUALIFIER tag)
{

    switch (tag)
    {
        case CONST_QUAL :
            msgn("const");
            break;
        case VOLATILE_QUAL :
            msgn("volatile");
            break;
        case CONST_VOL_QUAL :
            msgn("const volatile");
            break;
        case NO_QUAL :
            msgn("<no qual>");
            break;
        default :
            bug("illegal tag in \"ty_print_qualifier\"");
            break;
    }
}

/****************************************************************/
void ty_print_specifier(TYPE_SPECIFIER tag)
{

    switch (tag)
    {
        case INT_SPEC:
            msgn("int");
            break;
        case CHAR_SPEC:
            msgn("char");
            break;
        case FLOAT_SPEC:
            msgn("float");
            break;
        case DOUBLE_SPEC:
            msgn("double");
            break;
        case TYPENAME_SPEC:
            msgn("typename");
            break;
        case VOID_SPEC:
            msgn("void");
            break;
        case SIGNED_SPEC:
            msgn("signed");
            break;
        case UNSIGNED_SPEC:
            msgn("unsigned");
            break;
        case LONG_SPEC:
            msgn("long");
            break;
        case SHORT_SPEC:
            msgn("short");
            break;
        case CONST_SPEC:
            msgn("const");
            break;
        case VOLATILE_SPEC:
            msgn("volatile");
            break;
        case EXTERN_SPEC:
            msgn("extern");
            break;
        case STATIC_SPEC:
            msgn("static");
            break;
        case REGISTER_SPEC:
            msgn("register");
            break;
        case AUTO_SPEC:
            msgn("auto");
            break;
        case TYPEDEF_SPEC:
            msgn("typedef");
            break;
        case STRUCT_SPEC:
            msgn("struct");
            break;
        case UNION_SPEC:
            msgn("union");
            break;
        case ENUM_SPEC:
            msgn("enum");
            break;
        default:
            bug("illegal specifier in \"ty_print_specifier\"");
    }
}
/****************************************************************/
void ty_print_class(STORAGE_CLASS tag)
{

    switch (tag)
    {
        case EXTERN_SC:
            msgn("extern");
            break;
        case STATIC_SC:
            msgn("static");
            break;
        case REGISTER_SC:
            msgn("register");
            break;
        case AUTO_SC:
            msgn("auto");
            break;
        case TYPEDEF_SC:
            msgn("typedef");
            break;
        case NO_SC:
            msgn("<no storage class>");
            break;
        default:
            bug("illegal storage class in \"ty_print_class\"");
	     
    }
}

/****************************************************************/
void ty_print_dimflag(DIMFLAG tag)
{

    switch (tag)
    {
        case NO_DIM:
            msgn("no dimension");
            break;
        case DIM_PRESENT:
            msgn("dimension present");
            break;
        default:
            bug("illegal flag in \"ty_print_dimflag\"");
	     
    }
}

/****************************************************************/
void ty_print_paramstyle(PARAMSTYLE tag)
{

    switch (tag)
    {
        case OLDSTYLE:
            msgn("oldstyle");
            break;
        case DOT_DOT_DOT:
            msgn("elipsis prototype");
            break;
        default:
            bug("illegal tag in \"ty_print_paramstyle\"");
	     
    }
}

/****************************************************************/
void ty_print_paramlist(PARAM_LIST params)
{
    if (params == NULL)
    {
        msg("");
        msg("\t<parameter list is null>");
        return;
    }
    msgn(" (");
    while (params)
    {
        msg("");
            /* id might be NULL, but st_get_id_str handles NULL */
        msgn("\tparam: id %s; type ", st_get_id_str(params->id));
            /* type might be NULL, but ty_print_type handles NULL */
#ifdef PASCAL_LANG
        ty_print_type_pascal((TYPE) params->type);
#else
        if (params->is_ref)
            msgn("reference to ");
        ty_print_type((TYPE) params->type);
#endif
        params = params->next;
    }
    msg("");
    msgn("\t\t)");
}

/****************************************************************/
void ty_print_memlist(MEMBER_LIST members)
{

    if (members == NULL)
    {
        msg("");
        msg("\tmember list is <null>");
        return;
    }
    while (members != NULL)
    {
        msg("");
        msgn("\tmember: id %s; type ", st_get_id_str(members->id));
#ifdef PASCAL_LANG
	print_type_pascal(members->type, FALSE, members->type, FALSE);
#else
        print_type_c(members->type,FALSE);  /* don't recurse */
#endif
        members = members->next;
    }
    msg("");
}

/****************************************************************/
void ty_print_indexlist(INDEX_LIST indices)
{

    if (indices == NULL)
    {
	msgn("<no indices>");
	return;
    }

    while (indices != NULL)
    {
	msg("");
	msgn("\t\t\tindex: type ");
            /* don't recurse */
	print_type_pascal(indices->type, FALSE, indices->type, FALSE);
	indices = indices->next;
    }
    msg("");
}

/********************************************************************/
void print_type_pascal(TYPE typein, BOOLEAN recurse_flag,
                       TYPE slow, BOOLEAN cycle_check)
/* See the comments for ty_print_type */
{
    unsigned int length;
    PARAM_LIST params;
    INDEX_LIST indices;
    long low, high;
    TYPE object;
    TYPETAG tag;
    ST_ID id;
    BOOLEAN check_args;

    if (typein == NULL)
    {
	msgn("<null>");
	return;
    }

    if (cycle_check)
    {
	if (typein == slow)	/* cycle detected */
	{
	    msgn("...");
	    return;
	}
	slow = ty_strip_modifier(slow);
    }

    tag = typein->tag;

    switch (tag)
    {
        case TYARRAY:
            object = ty_query_array_pascal(typein,&indices);
            msgn("array [");
            ty_print_indexlist(indices);
            msgn("\t\t] of ");
            print_type_pascal(object,recurse_flag,slow,!cycle_check);
            break;

        case TYSET:
            object = ty_query_set(typein);
            msgn("set of ");
            print_type_pascal(object,recurse_flag,slow,!cycle_check);
            break;

        case TYPTR:
            object = ty_query_ptr_pascal(typein,&id);
            if (object == NULL)
                msgn("unresolved pointer to \"%s\"", st_get_id_str(id));
            else
            {  
                msgn("pointer to ");
                print_type_pascal(object,recurse_flag,slow,!cycle_check);
            }
            break;

        case TYSTRUCT:
            msg("struct ");
            if (recurse_flag)
                ty_print_memlist(typein->u.struct_union.members);
            break;

        case TYUNION:
            msg("union ");
            if (recurse_flag)
                ty_print_memlist(typein->u.struct_union.members);
            break;

        case TYENUM:
            msgn("enumeration ");
            break;

        case TYFUNC:
            object = ty_query_func_pascal(typein,&params,&check_args);
            if (ty_query(object) == TYVOID)
            {
                msgn("procedure");
                msgn(" <check args = %s>", check_args ? "YES" : "NO");
                ty_print_paramlist(params);
            }
            else
            {
                msgn("function");
                msgn(" <check args = %s>", check_args ? "YES" : "NO");
                ty_print_paramlist(params);
                msgn(" returning ");
                print_type_pascal(object,recurse_flag,slow,!cycle_check);
            }
            break;

        case TYSUBRANGE:
            object = ty_query_subrange(typein,&low,&high);
            msgn("subrange %ld .. %ld of ",low,high);
            ty_print_type_pascal(object);
            break;

        case TYFLOAT:
            msgn("float");
            break;

        case TYDOUBLE:
            msgn("double");
            break;

        case TYLONGDOUBLE:
            msgn("long double");
            break;

        case TYUNSIGNEDINT:
            msgn("unsigned int");
            break;

        case TYUNSIGNEDCHAR:
            msgn("unsigned char");
            break;

        case TYUNSIGNEDSHORTINT:
            msgn("unsigned short int");
            break;

        case TYUNSIGNEDLONGINT:
            msgn("unsigned long int");
            break;

        case TYSIGNEDCHAR:
            msgn("signed char");
            break;

        case TYSIGNEDINT:
            msgn("signed int");
            break;

        case TYSIGNEDLONGINT:
            msgn("signed long int");
            break;

        case TYSIGNEDSHORTINT:
            msgn("signed short int");
            break;

        case TYVOID:
            msgn("void");
            break;

        case TYERROR:
            msgn("error");
            break;

        default:
            bug("illegal typetag (%d) in \"ty_print_type_pascal\"", tag);
    }

}



/****************************************************************/
/*								*/
/*								*/
/*		All Following Routines will			*/
/*		be hidden from user of the module.		*/
/*								*/
/*								*/
/****************************************************************/




/****************************************************************/
/*								*/
/*								*/
/*		Storage Lists Management			*/
/*								*/
/*								*/
/****************************************************************/

/* global definitions for Storage List */

#define STORAGE_LIST_SIZE 500


/* list of encountered ptr types  */
static TYPE storage_list_ptr[STORAGE_LIST_SIZE]; 


/* list of encountered Array types  */
static TYPE storage_list_arr[STORAGE_LIST_SIZE]; 


/* list of encountered Function types  */
static TYPE storage_list_func[STORAGE_LIST_SIZE];


/* list of encountered Bitfield types  */
static TYPE storage_list_bitfield[STORAGE_LIST_SIZE];


static int storage_ptr = 0;   /* ptr to head of ptr storage list */

static int storage_arr = 0;   /* ptr to head of array storage list */

static int storage_func = 0;   /* ptr to head of func storage list */

static int storage_bitfield = 0;   /* ptr to head of bitfield storage list */

/****************************************************************/
static void add_storage_list(object,tag)

    TYPE object;
    TYPETAG tag;

		/* this routine adds a new type to the list of
		 previously encountered types */

    {
	switch (tag)
	    {
	    case TYPTR :
		if (storage_ptr < STORAGE_LIST_SIZE)
		    {
		    storage_list_ptr[storage_ptr] = object;
		    storage_ptr++;
		    }
		else
		  fatal("overflow in storage list ptr \"add_storage_list\"\n");
		break;

	    case TYARRAY :
		if (storage_arr < STORAGE_LIST_SIZE)
		    {
		    storage_list_arr[storage_arr] = object;
		    storage_arr++;
		    }
		else
		   fatal("overflow in storage list arr \"add_storage_list\"\n");
		break;

	    case TYFUNC :
		if (storage_func < STORAGE_LIST_SIZE)
		    {
		    storage_list_func[storage_func] = object;
		    storage_func++;
		    }
		else
		  fatal("overflow in storage list func \"add_storage_list\"\n");
		break;

	    case TYBITFIELD :
		if (storage_bitfield < STORAGE_LIST_SIZE)
		    {
		    storage_list_bitfield[storage_bitfield] = object;
		    storage_bitfield++;
		    }
		else
		  fatal("overflow in storage list bit  \"add_storage_list\"\n");
		break;

	    default :
		bug("Illegal tag in \"add_storage_list \" \n");
	    }	
    }


/*********************************************************************/
static TYPE check_object_func(object,paramstyle,params)

    TYPE object;
    PARAMSTYLE paramstyle;
    PARAM_LIST params;

			/*

			This routine checks whether a new function
			type already exists. If so, the existing structure
			will be shared with the new function type.

			This implementation assumes two function types
			are the same if:

				1. Parameter styles are the same.
				2. Function return types are the same.
				3. If parameter list style is "prototype"
				   or "elipsis", the parameter lists must
				   be the same length and each corresponding
				   parameter must have the same type.

			The definition of types being the "same" is not the
			same as "type equivalence". Structures may not be
			shared for two types that are equivalent (because
			of arrays without dimensions). Therefore,
			ty_test_equality is not used: pointer equivalence is
			used.
			*/


    {
    int index  = 0;
    TYPE *func = storage_list_func;
    BOOLEAN found = FALSE;


	    while ((index < storage_func) && !found)
		{
		if (((*func)->u.function.object != object) ||
		     ((*func)->u.function.paramstyle != paramstyle) ||
		     ((paramstyle != OLDSTYLE) &&
		      (!check_same_params(params,(*func)->u.function.params))))
		    {
		    index = index + 1;
		    func = storage_list_func + index;
		    }
		else
		    found = TRUE;
		 }
		if (found)
		    {
		    return(*func);
		    }
		else
		    return(NULL);

    }

/*********************************************************************/
static TYPE check_object_arr(object,dimflag,dimension)

    TYPE object;
    DIMFLAG dimflag;
    int dimension;

			/*

			This routine checks whether a new array
			type already exists. If so, the existing structure
			will be shared with the new array type.

			This implementation assumes two array types
			are the same if:

			1. Element types are the same.
			2. The dimflags are the same.
			3. If both dimflags are DIM_PRESENT, then the two
			   dimensions must also be the same.

			Note that this will not share structure for some
			equivalent arrays, namely in the case where one
			of the array dimensions is not present. This has
			to be treated as a special case in ty_test_equality.

			The definition of types being the "same" is not the
			same as "type equivalence". Structures may not be
			shared for two types that are equivalent (because
			of arrays without dimensions). Therefore,
			ty_test_equality is not used: pointer equivalence is
			used.
			*/

    {
    int index  = 0;
    TYPE *arr = storage_list_arr;
    BOOLEAN found = FALSE;


	    while ((index < storage_arr) && !found)
		{
		    if (((*arr)->u.array.object != object) ||
			((*arr)->u.array.dimflag != dimflag) ||
			((dimflag == DIM_PRESENT) &&
			 ((*arr)->u.array.dim != dimension)))
		    {
		    index++;
		    arr = storage_list_arr + index;
		    }
		else
		    found = TRUE;
		 }
		if (found)
		    {
		    return(*arr);
		    }
		else
		    return(NULL);
    }


/*********************************************************************/
static TYPE check_object_ptr(object,qualifier)

    TYPE object;
    TYPE_QUALIFIER qualifier;

			/*

			This routine checks whether a new pointer
			type already exists. If so, the existing structure
			will be shared with the new pointer type.

			This implementation assumes two pointer types
			are the same if:

			1. the types pointed to are the same.
			2. the qualifiers are the same.

			The definition of types being the "same" is not the
			same as "type equivalence". Structures may not be
			shared for two types that are equivalent (because
			of arrays without dimensions). Therefore,
			ty_test_equality is not used: pointer equivalence is
			used.

			*/
    {
    int index  = 0;
    TYPE *ptr = storage_list_ptr;
    BOOLEAN found = FALSE;


	    while ((index < storage_ptr) && !found)
		{
		if (((*ptr)->u.pointer.qual != qualifier) ||
		    ((*ptr)->u.pointer.object != object))
		    {
		    index = index + 1;
		    ptr = storage_list_ptr + index;
		    }
		else
		    found = TRUE;
		 }
	    if (found)
		{
		return(*ptr);
		}
	    else
		return(NULL);
    }


/*********************************************************************/
static TYPE check_object_bitfield(object,length)

    TYPE object;
    unsigned int length;

			/*

			This routine checks whether a new pointer
			type already exists. If so, the existing structure
			will be shared with the new pointer type.

			This implementation assumes two pointer types
			are the same if:

			1. the types of the bitfields are the same.
			2. the lengths of the bitfields are the same.

			The definition of types being the "same" is not the
			same as "type equivalence". Structures may not be
			shared for two types that are equivalent (because
			of arrays without dimensions). Therefore,
			ty_test_equality is not used: pointer equivalence is
			used.

			*/
    {
    int index  = 0;
    TYPE *bit = storage_list_bitfield;
    BOOLEAN found = FALSE;


	    while ((index < storage_bitfield) && !found)
		{
		if (((*bit)->u.bitfield.object != object) ||
		    ((*bit)->u.bitfield.length != length))
		    {
		    index++;
		    bit = storage_list_bitfield + index;
		    }
		else
		    found = TRUE;
		 }
		if (found)
		    return(*bit);
		else
		    return(NULL);
    }



/****************************************************************/
/*								*/
/*								*/
/*		Utility Routines				*/
/*								*/
/*								*/
/****************************************************************/

/*********************************************************************/
static BOOLEAN check_same_params(plist1,plist2)

    PARAM_LIST plist1;
    PARAM_LIST plist2;

		/*
		  This function returns true if the passed param
		  lists are the same, false otherwise; param lists
		  are considered the same if they have the same number
		  of elements and each coresponding param is of the
		  same type.

  		  The definition of types being the "same" is not the
		  same as "type equivalence". Structures may not be
		  shared for two types that are equivalent (because
		  of arrays without dimensions). Therefore,
		  ty_test_equality is not used: pointer equivalence is
		  used.
		 */

    {

	/*
	run through the two lists looking for two corrspeonding
	types that are not the same. note that the order of the
	list does not matter. however, the next field of the last
	node in the linked list must be NULL.
	*/
	while ((plist1 != NULL) && (plist2 != NULL))
	{
		if (plist1->type != plist2->type)
			return FALSE;
		plist1 = plist1->next;
		plist2 = plist2->next;
	}
	/* 
	loop exits when one of the pointers goes to null;
	for parameter lists to be the same length both pointers
	must be null. if both are null, then parameter lists are
	the same since all parameter types have now been checked.
	*/
	return (plist1 == plist2);
    }

static BOOLEAN check_params_equality(plist1,plist2)

    PARAM_LIST plist1;
    PARAM_LIST plist2;

		/*
		  This function returns true if the passed param
		  lists are equal, false otherwise; param lists
		  are considered equal if they have the same number
		  of elements and each coresponding param is of 
		  equal type.

		  Note: type equality is being tested here (not sameness),
		  so ty_test_equality is used.
		 */

    {

	/*
	run through the two lists looking for two corrspeonding
	types that are not equal. note that the order of the
	list does not matter. however, the next field of the last
	node in the linked list must be NULL.
	*/
	while ((plist1 != NULL) && (plist2 != NULL))
	{
		if (!ty_test_equality((TYPE) plist1->type, (TYPE) plist2->type))
			return FALSE;
		plist1 = plist1->next;
		plist2 = plist2->next;
	}
	/* 
	loop exits when one of the pointers goes to null;
	for parameter lists to be the same length both pointers
	must be null. if both are null, then parameter lists are
	equal since all parameter types have now been checked.
	*/
	return (plist1 == plist2);
	
    }


/**************************************************************/
static BOOLEAN test_equality(TYPE type1, TYPE type2, int ptr_cnt)
/* (Pascal only)
  Type equivalence in Pascal is not defined very well, and varies
  with the dialect.  It is always some combination of structural
  and name equivalence.  We strike a compromise here by checking
  structural equivalence on type structures with the following
  exceptions:

  1. occurrences of arrays, records, and enumerated types are
  considered their own unique types (we don't recurse through
  them but use pointer equality instead).

  2. we only recurse through at most two levels of TYPTR nodes,
  resorting to pointer equality for any deeper level.  This
  avoids endlessly chasing a cycle in the type structure
  (which must go through at least one TYPTR node).
  (Doing full structural equivalence would require
  maintaining an auxillary data structure.)  Also, we don't
  recurse through pointer types at all when processing
  procedure/function parameter types.

  We maintain the number of available TYPTR nodes to recurse through
  in the parameter ptr_cnt.  Initially, this value is set to 2
  in ty_test_equality.
*/
{
    long l1, l2, h1, h2;
    TYPETAG tag1, tag2;
    TYPE obj1, obj2;
    PARAM_LIST pl1, pl2;
    BOOLEAN ca1, ca2;

    if (type1 == NULL || type2 == NULL)
    {
	bug("Null argument(s) to \"ty_test_equality\"");
	return FALSE;
    }

#if 0
        /* Subranges over the same base type are compatible with the base
           type and with each other, so just substitute the base type
           for the subrange type. */
    if (ty_query(type1) == TYSUBRANGE)
	type1 = ty_strip_modifier(type1);
    if (ty_query(type2) == TYSUBRANGE)
	type2 = ty_strip_modifier(type2);
#endif

    tag1 = ty_query(type1);
    tag2 = ty_query(type2);

    if (tag1 == TYERROR || tag2 == TYERROR)
	fatal("TYERROR encountered in \"ty_test_equality\"");

    if (tag1 == TYBITFIELD || tag2 == TYBITFIELD)
	fatal("Bit fields are not supported in Standard Pascal.");

        /* If not the same type tag */
    if (tag1 != tag2)
	return FALSE;

        /* Type tags are equal at this point. */
    obj1 = ty_strip_modifier(type1);
    obj2 = ty_strip_modifier(type2);
    switch (tag1)
    {
        case TYPTR:
                /* Check resolved versus unresolved */
            if (obj1 == NULL && obj2 == NULL)
                return type1->u.pascal_pointer.id == type2->u.pascal_pointer.id;
            if (obj1 == NULL || obj2 == NULL)
                return FALSE;
            if (ptr_cnt > 0)	/* Recurse through the pointer type */
                return test_equality(obj1, obj2, ptr_cnt - 1);
                /* Otherwise, fall through! */
        case TYARRAY:
        case TYSTRUCT:
        case TYUNION:
        case TYENUM:
                /* Treat these as unique types */
            return type1 == type2;

        case TYSUBRANGE:
#if 0
            return obj1 == obj2;
#else
            (void) ty_query_subrange(type1,&l1,&h1);
            (void) ty_query_subrange(type2,&l2,&h2);
            return obj1 == obj2 && l1 == l2 && h1 == h2;
#endif

        case TYSET:
            return test_equality(obj1, obj2, ptr_cnt);

        case TYFUNC:
                /* First, check return types. */
            if (!test_equality(obj1,obj2,ptr_cnt))
                return FALSE;
                /* Now check the parameter lists in order.  (Don't recurse
                   through pointers in parameter types, i.e., set ptr_cnt = 0)*/
            (void) ty_query_func_pascal(type1,&pl1,&ca1);
            (void) ty_query_func_pascal(type2,&pl2,&ca2);
            if (!ca1 || !ca2)	/* If args not checked */
                return TRUE;
                /* Check params */
            while (pl1 != NULL && pl2 != NULL)
            {
                if (pl1->is_ref != pl2->is_ref ||
                    pl1->sc != pl2->sc ||
                    !test_equality(pl1->type,pl2->type,0))
                    return FALSE;
                pl1 = pl1->next;
                pl2 = pl2->next;
            }
                /* Return TRUE iff both parameter list pointers are NULL. */
            return pl1 == pl2;
	
                /* The rest of the tags are for basic types, and these tags
                   determine their types completely. */
        default:
            return TRUE;
    }

    bug("How did I get here?");
}
