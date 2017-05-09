/****************************************************************************/
/* 
	definitions to support types.c
*/

#ifndef TYPES_H
#define TYPES_H

#include "defs.h"

/* "users" should not need to know details of type representation */
typedef struct type_rec *TYPE;

/* can be used in array TYPE */
typedef enum {NO_DIM, DIM_PRESENT} DIMFLAG;

/* returned by ty_query */
typedef enum {
    TYVOID, TYFLOAT, TYDOUBLE, TYLONGDOUBLE, TYSIGNEDLONGINT,
    TYSIGNEDSHORTINT, TYSIGNEDINT, TYUNSIGNEDLONGINT,
    TYUNSIGNEDSHORTINT, TYUNSIGNEDINT, TYUNSIGNEDCHAR,
    TYSIGNEDCHAR, TYSTRUCT, TYUNION, TYENUM, TYARRAY, TYSET,
    TYFUNC, TYPTR, TYBITFIELD, TYSUBRANGE, TYERROR
} TYPETAG;

/* used in function TYPE */
typedef enum {PROTOTYPE, OLDSTYLE, DOT_DOT_DOT} PARAMSTYLE;

/* compact representation of TYPE qualifiers */
typedef enum {
    NO_QUAL, CONST_QUAL, VOLATILE_QUAL, CONST_VOL_QUAL
} TYPE_QUALIFIER;

/* compact representation of storage class specifiers */
typedef enum {
    STATIC_SC, EXTERN_SC, AUTO_SC, TYPEDEF_SC, REGISTER_SC, NO_SC
} STORAGE_CLASS;

/*
Parameter list structure IS manipulated directly by "users". types.c
makes no assumption about the parameter list except all PARAMs should
be linked together by the "next" field, and the last PARAM in that linked
list should have a NULL "next" field. In particular, the order of the
PARAMS is flexible and "prev" field can be used for any purpose (types.c
doesn't use it).
*/
typedef struct param {
    ST_ID id;
    TYPE type;
    STORAGE_CLASS sc;
    TYPE_QUALIFIER qu;
    BOOLEAN err;
    BOOLEAN is_ref;
    struct param *next, *prev;
} PARAM, *PARAM_LIST;

/*
Member list structure IS manipulated directly by "users". types.c
makes no assumption about the member list except all MEMBERS should
be linked together by the "next" field, and the last MEMBER in that linked
list should have a NULL "next" field. In particular, the order of the
MEMBERS is flexible and "prev" field can be used for any purpose (types.c
doesn't use it).
*/
typedef struct member {
    ST_ID id;
    TYPE type;
    STORAGE_CLASS sc;
    TYPE_QUALIFIER qu;
    BOOLEAN err;
    struct member *next, *prev;
} MEMBER, *MEMBER_LIST;

/*
Type list structure IS manipulated directly by "users".  types.c only uses
this structure to hold index types of arrays in Pascal (see below).
Otherwise, this structure is available to use however one wishes; for example,
it can be used to hold a list of unresolved pointer types in Pascal.
*/
typedef struct tlist {
    TYPE type;
    struct tlist *next, *prev;
} TLIST_NODE, *TYPE_LIST;

/*
Index list structure IS manipulated directly by "users".
types.c only uses type lists to hold index types of arrays in Pascal, using
the renamed INDEX and INDEX_LIST.  types.c makes no assumption about such an
index list except all INDEXes should be linked together by the "next" field,
and the last INDEX in that linked list should have a NULL "next" field.  In
particular, the order of the INDEXes is flexible and "prev" field can be used
for any purpose (types.c doesn't use it).
*/
typedef struct tlist INDEX, *INDEX_LIST;

/*
The type specifiers: i.e., pieces of a type declaration. That is, there is a
difference between the specifier "float" (just a piece of the
declaration "float **x" perhaps) and the TYPE float!
What we are calling a TYPE_SPECIFIER here also includes type qualifiers
(e.g., "const") and storage class specifiers (e.g., "static") as well as
actual type specifiers (e.g., "int").
*/
typedef enum {
    VOLATILE_SPEC, CONST_SPEC, MONO_SPEC, POLY_SPEC,
    SIGNED_SPEC, UNSIGNED_SPEC, SHORT_SPEC, LONG_SPEC,
    INT_SPEC, FLOAT_SPEC, DOUBLE_SPEC, CHAR_SPEC, VOID_SPEC,
    STRUCT_SPEC, UNION_SPEC, ENUM_SPEC, TYPENAME_SPEC,
    STATIC_SPEC, EXTERN_SPEC, AUTO_SPEC, TYPEDEF_SPEC,
    REGISTER_SPEC
} TYPE_SPECIFIER;


/*
The following are just a set of macros for separating out type specifiers.
*/
#define IS_BASE(typein)	( (typein == INT_SPEC) || (typein == CHAR_SPEC) ||\
			  (typein == VOID_SPEC) || (typein == FLOAT_SPEC) ||\
			  (typein == DOUBLE_SPEC) || (typein == STRUCT_SPEC) ||\
			  (typein == UNION_SPEC) || (typein == ENUM_SPEC) ||\
			  (typein == TYPENAME_SPEC) )

#define IS_QUAL(typein)	( (typein == CONST_SPEC) || (typein == VOLATILE_SPEC) )

#define IS_SIZE(typein)	( (typein == SHORT_SPEC) || (typein == LONG_SPEC) )

#define IS_SIGN(typein)	( (typein == UNSIGNED_SPEC) || (typein == SIGNED_SPEC) )

#define IS_CLASS(typein) ((typein == EXTERN_SPEC) || (typein == STATIC_SPEC) ||\
			  (typein == REGISTER_SPEC) || (typein == AUTO_SPEC) ||\
			  (typein == TYPEDEF_SPEC) )


/* Routines to manipulate "TYPE" */

void ty_types_init();
    /*
    This routine initializes the types module data structures.
    */


TYPETAG ty_query(TYPE type);
    /*
    This routine returns the actual type or tag value of the "TYPE". (PROJ1)
    */


BOOLEAN ty_test_equality(TYPE type1, TYPE type2);
    /*
    This routine takes 2 parameters both of type "TYPE"
    and returns TRUE if the 2 TYPES are equal (compatible)
    and FALSE otherwise.
    */

TYPE ty_strip_modifier(TYPE typein);
    /*
    This routine takes 1 parameter of type "TYPE". If the 
    tag of the type is a modifier (e.g., ptr,array,func,set,subrange)
    the routine will strip off the modifier and return
    the remainder of the type. If the type sent in is not 
    a modifier, it returns NULL.
    */

TYPE ty_build_basic(TYPETAG tag);
    /*
    This routine takes 1 parameter, a type tag of a basic type
    It builds a "TYPE" out of the basic tag passed in.  (PROJ1)
    */

TYPE ty_build_ptr_pascal(TYPE object);
    /*
    This routine takes 1 parameter, the "TYPE" to be pointed to. In Pascal,
    if the type is declared by the user, then this will be either a
    procedure or function type, but in general it could be any type.
    ty_build_ptr constructs a new pointer TYPE with object as the object
    type.
    */

TYPE ty_build_ptr(TYPE object, TYPE_QUALIFIER qual);
    /*
    This routine takes 2 parameters, a "TYPE" and a qualifier.
    It constructs a new TYPE which it returns which will be "ptr to object".
    Pointer can be "const" or "volatile" which the second parameter
    specifies.
    */

TYPE ty_build_unresolved_ptr(ST_ID id);
    /* (Pascal only)
    This routine takes 1 parameter, the enrollment papers for an unresolved
    identifier to be pointed to.  ty_build_unresolved_ptr constructs and
    returns a new pointer TYPE whose object type is NULL.  The identifier
    is included with the new unresolved pointer type.  Every unresolved
    pointer type should eventually be resolved to point to an actual TYPE
    by calling ty_resolve_ptr().
    */

TYPE ty_build_array_pascal(TYPE object, INDEX_LIST indices);
    /* (Pascal only)
    This routine takes 2 parameters: a "TYPE", and a list of indices
    of the array.  It constructs a new TYPE which it returns which will
    contain the array indices and be "array of object".  (PROJ1)
    */

TYPE ty_build_array(TYPE object, DIMFLAG dimflag, int dimension);
    /* (C/C++ only)
    This routine takes 3 parameters: a "TYPE", a flag indicating whether a 
    dimension is available, and the dimension of the array. The routine 
    constructs a new TYPE which it returns and which will contain the array 
    dimension (if there is one) and be "array of object".
    */

TYPE ty_build_set(TYPE object);
    /* (Pascal only)
    This routine takes 1 parameter: the "TYPE" of the elements of a set.
    It returns a new TYPE which will be "set of object".
    */

TYPE ty_build_struct(ST_ID tagname, MEMBER_LIST members);
    /*
    This routine takes 2 parameters (a tagname and a member list
    and returns a TYPE for the structure containing
    its member list. If there is no tagname, then the first argument
    should be NULL.
    */

TYPE ty_build_union(ST_ID tagname, MEMBER_LIST members);
    /*
    This routine takes 3 parameters (a tagname, a member list, and
    a parallel qualifier), and returns a TYPE for the union containing
    its member list. If there is no tagname, then the first argument
    should be NULL.
    */

TYPE ty_build_enum_pascal(unsigned int num_values);
    /* (Pascal only)
    This routine takes one argument, the number of values of an
    enumerated type, and returns the TYPE enumeration.
    */

TYPE ty_build_enum(ST_ID tagname);
    /* (C/C++ only)
    This routine takes one argument, a tagname, and returns a TYPE
    enumeration.
    */

TYPE ty_build_bitfield(TYPE object, unsigned int length);
    /*
    This routine builds and returns a bitfield type storing the
    length of the bitfield.
    */

TYPE ty_build_func_pascal(TYPE object, PARAM_LIST params, BOOLEAN check_args);
    /* (Pascal only)
    This routine takes 3 parameters: a previously constructed
    object of type "TYPE", a parameter list for the function, and a
    BOOLEAN flag whether to check that actual arguments correspond to
    formal arguments in a function call.  The routine builds a "TYPE"
    for the function and stores the functions parameters and flag.
    It returns the newly constructed type.  (PROJ1)
    */

TYPE ty_build_func(TYPE object, PARAMSTYLE paramstyle, PARAM_LIST params);
    /* (C/C++ only)
    This routine takes 3 parameters: a previously constructed object of
    type "TYPE", a parameter list for the function (3rd param), and a flag
    indicating what kind of parameter list is present (2nd param).  The
    routine builds a "TYPE" for the function and stores the function's
    parameters. It returns the newly constructed type.
    */

TYPE ty_build_subrange(TYPE object, long low, long high);
    /* (Pascal only)
    This routine takes 3 parameters: a costructed object of type "TYPE"
    (the base type of the subrange to be constructed--an ordinal type),
    the ordinal value of the lower limit, and the ordinal value of the
    upper limit of the subrange.  The routine builds and returns
    a constructed subrange type from the values given.  (PROJ1)
    */

TYPE ty_query_func_pascal(TYPE type, PARAM_LIST *params, BOOLEAN *check_args);
    /* (Pascal only)
    This routine takes one input parameter which must be a function TYPE.
    The routine returns through the second parameter the parameter list
    itself, and through the third parameter the argument check flag.
    The function return value is the TYPE of the return value of the
    input function type.
    */

TYPE ty_query_func(TYPE type, PARAMSTYLE *paramstyle, PARAM_LIST *params);
    /* (C/C++ only)
    This routine takes one input parameter which must be a function TYPE.
    The routine returns through the parameter list the parameter style,
    the parameter list itself, and the function qualifier (sync, parallel,
    or pure). The function return value is the TYPE
    of the return value of the input function type.
    */

TYPE ty_query_array_pascal(TYPE type, INDEX_LIST *indices);
    /* (Pascal only)
    This routine takes 1 input parameter which must be an array TYPE.
    It returns through the second parameter the indices of the
    array.  Through the function return value it returns the type of
    the array elements.
    */

TYPE ty_query_array(TYPE type, DIMFLAG *dimflag, unsigned int *dim);
    /* (C/C++ only)
    This routine takes 1 input parameter which must be an array TYPE.
    It returns through the parameter list the flag that indicates whether
    a dimension is present. Also through the parameter list it returns the
    dimension. Through the function return value it returns the type of
    the array elements.
    */

TYPE ty_query_bitfield(TYPE type, unsigned int *length);
    /*
    This routine takes 1 input parameter which must be a bitfield type.
    It returns through the parameter list the length of the bitfield.
    It returns as the function return value the TYPE of the bitfield.
    */

TYPE ty_query_set(TYPE type);
    /* (Pascal only)
    This routine takes 1 parameter which must be a set TYPE.
    It returns the element type of the set.
    */

TYPE ty_query_ptr_pascal(TYPE type, ST_ID *id);
    /* (Pascal only)
    This routine takes 1 input parameter "type" which must be a pointer
    type node.  It returns the TYPE pointed to, if there is one.  If not,
    the object type is unresolved, and NULL is returned.  In the case of
    an unresolved pointer type, the output parameter id is set to the
    original enrollment papers passed to ty_build_unresolved_ptr() when
    the pointer was first created.  If the pointer is resolved, the id
    parameter is set to NULL.
    (PROJ1)
    */

TYPE ty_query_pointer(TYPE type, TYPE_QUALIFIER *qual);
    /* (C/C++ only)
    This routine takes 1 input parameter which must be a pointer type.
    It returns through the parameter list the type_qualifier of the
    pointer type. It returns as the function return value the TYPE pointed to.
    */

TYPE ty_query_subrange(TYPE type, long *low, long *high);
    /* (Pascal only)
    This routine takes 1 input parameter which must be a subrange
    type.  It returns through the parameter list the ordinal values
    of the lower and upper limits of the subrange.  It returns as
    the function return value the base type of the subrange.  (PROJ1)
    */

unsigned int ty_query_enum(TYPE type);
    /* (Pascal only)
    This routine takes 1 parameter, which must be an enumerated type.
    It returns the number of values in the enumeration.
    */

TYPE ty_assign_members(TYPE type, MEMBER_LIST members);
    /*
    This routine takes 2 parameters: a previously constructed
    "TYPE" and a struct or union member list. If the input type
    is of type struct, union or domain, the routine assigns the member
    list to the input type. If the type is not one of these types
    it is an error.
    */

MEMBER_LIST ty_retrieve_members(TYPE type);
    /*
    This routine takes 1 parameter which must be a struct or union TYPE.
    It returns the member list of the given struct/union.
    */

ST_ID ty_retrieve_tagname(TYPE typein);
    /*
    This routine takes 1 parameter which must be either struct, union,
    or enum TYPE. It returns the tagname of the given struct/union/enum.
    */

BOOLEAN ty_resolve_ptr(TYPE ptr_type, TYPE object_type);
    /* (Pascal only)
    This routine takes 2 parameters: a pointer-to-unresolved type (ptr_type)
    and the new resolved type (object_type).  Changes the object type
    of the pointer to the new type.  Returns TRUE if successful, and
    FALSE if there is a problem (nonpointer, NULL object_type, etc.)
    (PROJ1)
    */

void ty_print_type_pascal(TYPE typein);
void ty_print_type(TYPE typein);
    /*
    This function takes 1 parameter, a TYPE, and just prints out in
    English the meaning of the TYPE.  Behavior varies with source language.
    */

void ty_print_class(STORAGE_CLASS sc);
    /*
    This function takes 1 parameter, a storage class, and just prints out in
    English the meaning of the "C" storage class.
    */

void ty_print_qualifier(TYPE_QUALIFIER qualifier);
    /*
    This function takes 1 parameter, a qualifier, and just prints out
    in English the meaning of the "C" TYPE qualifier.
    */

void ty_print_specifier(TYPE_SPECIFIER specifier);
    /*
    This function takes 1 parameter, a specifier, and just prints out
    in English the meaning  of the "C" TYPE specifier.
    */

void ty_print_typetag(TYPETAG tag);
    /*
    This function takes 1 parameter, a TYPE tag, and just prints out
    in English the meaning of the TYPE tag.
    */

void ty_print_dimflag(DIMFLAG dimflag);
    /*
    This function takes 1 parameter, an array TYPE dimension flag, and
    just prints out in English the meaning of the dimension flag.
    */

void ty_print_paramlist(PARAM_LIST params);
    /*
    This function takes 1 parameter, a pointer to a function TYPE prototype
    parameter list, and prints out in English the meaning of each parameter
    of that list.
    */

void ty_print_memlist(MEMBER_LIST params);
    /*
    This function takes 1 parameter, a pointer to a struct/union TYPE member
    list, and prints out in English the meaning of each member of that list.
    */

void ty_print_paramstyle(PARAMSTYLE paramstyle);
    /*
    This function takes 1 parameter, a function TYPE parameter style flag,
    and prints out in English the meaning of the flag.
    */

void ty_print_indexlist(INDEX_LIST indices);
    /* (Pascal only)
    This function takes 1 parameter, a pointer to a list of array indices,
    and prints out in English the meaning of each index of that list.
    */

/***************************************************/
/* Utilities (defined in utils.c)                  */
/***************************************************/

unsigned int get_size_basic(TYPETAG tag);
    /*
    This function takes 1 parameter, a typetag, and returns the size
    (in bytes) of the basic type corresponding to the typetag.
    */

PARAM_LIST plist_alloc();
    /*
    Returns a pointer to a newly allocated PARAM_LIST node
    */

void plist_free(PARAM_LIST plist);
    /*
    De-allocates a single PARAM_LIST node (plist).  Does NOT deallocate
    an entire list.
    */

#endif
