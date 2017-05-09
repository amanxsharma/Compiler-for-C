/* BUCKET.c module

The routines in this module were stripped from builtins.c and placed in their
own file.

Routine update: 5/27/89

The bucket record definition was modified to use masks and bitwise operations
instead of arrays.  All the routines in this module reflect that change.

Masks are calculated by using declaration specifiers as bit positions within
a 32 bit integer.  The definition for these specifiers uses enumerations
numbered from zero through the maximum number of specifiers (currently 24).
Masks are created by left shifting 1 over by the value of the specifier
enumeration + 1.  Because the declaration specifiers are defined in a 
particluar order (ie. all size specifers, then all sign specifiers, etc.) it
is important to keep them in that order.  Evaluation of the complete
declaration type involves looking at which bits are turned on within the
appropriate bucket record field.  See the comments in "types.h" on how to add
declartion specifiers to this definition. */

#include <stdlib.h>
#include "defs.h"
#include "types.h"
#include "bucket.h"
#include "message.h"

#define FIRST_BASE      (int) INT_SPEC
#define FIRST_SIGN	(int) SIGNED_SPEC
#define FIRST_SIZE	(int) SHORT_SPEC
#define FIRST_CLASS	(int) STATIC_SPEC
#define FIRST_QUAL	(int) VOLATILE_SPEC

/*
number of different types of type specifiers.
must agree with definition of TYPE_SPECIFIER in "defs.h"
*/

#define NUM_BASE  9  /* int,float,double,char,struct,union,enum,void,typename */
#define NUM_SIZE  2  /* short,long */
#define NUM_SIGN  2  /* unsigned,signed */
#define NUM_QUAL  2  /* const,volatile */
#define NUM_CLASS 5  /* static,extern,register,auto,typedef */

/* masks used when building bucket record */

static unsigned BASE_MASK;
static unsigned CLASS_MASK;
static unsigned SIGN_MASK;
static unsigned SIZE_MASK;
static unsigned QUAL_MASK;

static unsigned VOLATILE_MASK, CONST_MASK, STATIC_MASK;
static unsigned EXTERN_MASK, AUTO_MASK, TYPEDEF_MASK, REGISTER_MASK, LONG_MASK;
static unsigned SHORT_MASK, SIGNED_MASK, UNSIGNED_MASK, INT_MASK, CHAR_MASK;
static unsigned VOID_MASK, FLOAT_MASK, DOUBLE_MASK;

static unsigned LONGDOUBLE_MASK, SIGNEDLONGINT_MASK, SIGNEDSHORTINT_MASK;
static unsigned SIGNEDINT_MASK, UNSIGNEDLONGINT_MASK, UNSIGNEDSHORTINT_MASK;
static unsigned UNSIGNEDINT_MASK, UNSIGNEDCHAR_MASK,SIGNEDCHAR_MASK,STRUCT_MASK;
static unsigned UNION_MASK, ENUM_MASK, TYPENAME_MASK;

/*****************************************************************************/

/* function INIT_BUCKET_MODULE

	This routine calculates all the masks used throughout this module */

void init_bucket_module (void)

{
	int i;

	for (i = FIRST_BASE; i < FIRST_BASE + NUM_BASE; i++)
		BASE_MASK |= 1 << (i + 1);

	for (i = FIRST_CLASS; i < FIRST_CLASS + NUM_CLASS; i++)
		CLASS_MASK |= 1 << (i + 1);

	for (i = FIRST_SIGN; i < FIRST_SIGN + NUM_SIGN; i++)
		SIGN_MASK |= 1 << (i + 1);

	for (i = FIRST_SIZE; i < FIRST_SIZE + NUM_SIZE; i++)
		SIZE_MASK |= 1 << (i + 1);

	for (i = FIRST_QUAL; i < FIRST_QUAL + NUM_QUAL; i++)
		QUAL_MASK |= 1 << (i + 1);

	VOLATILE_MASK = (1 << (VOLATILE_SPEC+1));
	CONST_MASK = (1 << (CONST_SPEC+1));
	STATIC_MASK = (1 << (STATIC_SPEC+1));
	EXTERN_MASK = (1 << (EXTERN_SPEC+1));
	AUTO_MASK = (1 << (AUTO_SPEC+1));
	TYPEDEF_MASK = (1 << (TYPEDEF_SPEC+1));
	REGISTER_MASK = (1 << (REGISTER_SPEC+1));
	LONG_MASK = (1 << (LONG_SPEC+1));
	SHORT_MASK = (1 << (SHORT_SPEC+1));
	SIGNED_MASK = (1 << (SIGNED_SPEC+1));
	UNSIGNED_MASK = (1 << (UNSIGNED_SPEC+1));
	INT_MASK = (1 << (INT_SPEC+1));
	CHAR_MASK = (1 << (CHAR_SPEC+1));
	VOID_MASK = (1 << (VOID_SPEC+1));
	FLOAT_MASK = (1 << (FLOAT_SPEC+1));
	DOUBLE_MASK = (1 << (DOUBLE_SPEC+1));
	LONGDOUBLE_MASK = LONG_MASK | DOUBLE_MASK;
	SIGNEDLONGINT_MASK = SIGNED_MASK | LONG_MASK | INT_MASK;
	SIGNEDSHORTINT_MASK = SIGNED_MASK | SHORT_MASK | INT_MASK;
	SIGNEDINT_MASK = SIGNED_MASK | INT_MASK;
	UNSIGNEDLONGINT_MASK = UNSIGNED_MASK | LONG_MASK | INT_MASK;
	UNSIGNEDSHORTINT_MASK = UNSIGNED_MASK | SHORT_MASK | INT_MASK;
	UNSIGNEDINT_MASK = UNSIGNED_MASK | INT_MASK;
	UNSIGNEDCHAR_MASK = UNSIGNED_MASK | CHAR_MASK;
	SIGNEDCHAR_MASK = SIGNED_MASK | CHAR_MASK;
	STRUCT_MASK = (1 << (STRUCT_SPEC+1));
	UNION_MASK = (1 << (UNION_SPEC+1));
	ENUM_MASK = (1 << (ENUM_SPEC+1));
	TYPENAME_MASK = (1 << (TYPENAME_SPEC+1));
}	

/**************************************************************/
void print_bucket(BUCKET_PTR bucket)

    {
	char *show_bucket();
	msgn (show_bucket(bucket));
    }


char *show_bucket(BUCKET_PTR bucket)

    {
	if (bucket)
	    return("<non-null bucket>");
	else
	    return("<null bucket>");
    }

/*B***************************************************************/
BUCKET_REC *update_bucket(BUCKET_PTR bucket,TYPE_SPECIFIER spec,TYPE type)


		 /* this routine take in pointer to the type
		  information bucket and also the specifier
		  to be added to the bin.  If there is not
		  something already there it adds it to the bin
		  else an error.

		  the third parameter is usually NULL. With enumerations,
		  structures, and unions the third field has a type in it.
		  With typenames, the third parameter is an id to be looked
		  up.
		 */
    {
    unsigned int mask;

    if (bucket == NULL)
	{
	if ((bucket = (BUCKET_REC *)calloc(1,sizeof(BUCKET_REC))) == NULL)
	    bug("Ran out of mem in update_bucket");

	bucket->spec_def = 0;
	}

    if (bucket->type == NULL)
	bucket->type = type; /* only remembers last one; error caught below */

    mask = 1 << ((int) spec + 1);	/* specifier mask */

    if (IS_BASE(spec))
	{
	if ((bucket->spec_def & LONG_MASK) && spec == DOUBLE_SPEC &&
	     (bucket->spec_def & SIGN_MASK) == 0)
	    bucket->spec_def |= mask;
	else if ((bucket->spec_def & BASE_MASK) || 
	    ((bucket->spec_def & SIZE_MASK) && spec != INT_SPEC) || 
	    ((bucket->spec_def & SIGN_MASK) && spec != INT_SPEC &&
	    spec != CHAR_SPEC))
	    {
	    bucket->error_decl = TRUE;

	    if (spec == TYPENAME_SPEC)
		error ("illegal type specifier in declaration");
	    else
	    	error ("illegal type specifier");
	    }
	else
	    bucket->spec_def |= mask;
	}
    else if (IS_SIGN(spec))
	{
	if ((bucket->spec_def & SIGN_MASK) || 
	  ((bucket->spec_def & FLOAT_MASK) || (bucket->spec_def & DOUBLE_MASK)))
       	    {
	    bucket->error_decl = TRUE;
	    error ("illegal sign specifier");
	    }
	else
	    bucket->spec_def |= mask;
	}
    else if (IS_SIZE(spec))
	{
	if ((bucket->spec_def & SIZE_MASK) || ((bucket->spec_def & CHAR_MASK) ||
           (bucket->spec_def & FLOAT_MASK) || 
	   ((bucket->spec_def & DOUBLE_MASK) && spec != LONG_SPEC)))
	    {
	    bucket->error_decl = TRUE;
	    error ("illegal size specifier");
     	    }
	else
	    bucket->spec_def |= mask;
	}
    else if (IS_QUAL(spec))
	{
	if (((bucket->spec_def & VOLATILE_MASK) && spec == CONST_SPEC) ||
	    ((bucket->spec_def & CONST_MASK) && spec == VOLATILE_SPEC)) 
	    bucket->spec_def |= mask;
	else if (bucket->spec_def & QUAL_MASK)	
	    {
	    bucket->error_decl = TRUE;
	    error ("illegal type qualifier");
	    }
	else
	    bucket->spec_def |= mask;
	}
    else if (IS_CLASS(spec))
	{
	if (bucket->spec_def & CLASS_MASK)	
	    {
	    bucket->error_decl = TRUE;
	    error ("illegal storage class specifier");
	    }
	else
	    bucket->spec_def |= mask;
	}
    else
	bug ("illegal specifier in update_bucket");

    return bucket;
    }

/******************************************************************/

STORAGE_CLASS get_class (BUCKET_PTR bucket)

	/* this routine takes in the type info bucket and return the
	  storage class of the mask
	 */

{
	if (bucket == NULL)
	    return NO_SC;
	else if (bucket->spec_def & STATIC_MASK)
	    return(STATIC_SC);
	else if (bucket->spec_def & EXTERN_MASK)
	    return(EXTERN_SC);
	else if (bucket->spec_def & AUTO_MASK)
	    return(AUTO_SC);
	else if (bucket->spec_def & REGISTER_MASK)
	    return(REGISTER_SC);
	else if (bucket->spec_def & TYPEDEF_MASK)
	    return(TYPEDEF_SC);
	else
	    return(NO_SC);
    }

/*B**************************************************************/
TYPE_QUALIFIER get_qual(BUCKET_PTR bucket)

	/* this routine takes in the type info bucket and return the
	  type qualifier(s) of the mask
	 */

    {
	if (((bucket->spec_def & CONST_MASK) && 
	    (bucket->spec_def & VOLATILE_MASK)))
	    return(CONST_VOL_QUAL);
	else if (bucket->spec_def & CONST_MASK)
	    return(CONST_QUAL);
	else if (bucket->spec_def & VOLATILE_MASK)
	    return(VOLATILE_QUAL);
	else
	    return(NO_QUAL);
    }

/*B************************************************************/
BOOLEAN is_error_decl(BUCKET_PTR bucket)

	/* this function returns TRUE is there was an error in the
	  declaration of the base type else returns FALSE
	 */

    {
    if (bucket != NULL)
	{
	if (bucket->error_decl == TRUE)
	    return(TRUE);
	else
	    return(FALSE);
	}

    bug("Null bucket in is_error_decl");
    }


/*B************************************************************/
TYPE build_base (BUCKET_PTR bucket)

	/* this routine takes in a bucket of type info and 
	  build a base type from the info specified.
	 */


    {
    TYPE type;

	if ((bucket->spec_def & BASE_MASK) == 0)
	    bucket->spec_def |= INT_MASK;

	if ((bucket->spec_def & SIGN_MASK) == 0)
	    {
	    if ((bucket->spec_def & INT_MASK) || (bucket->spec_def & CHAR_MASK))
	    	bucket->spec_def |= SIGNED_MASK;
	    }

	if ((bucket->spec_def & FLOAT_MASK)==FLOAT_MASK)
	    type = ty_build_basic(TYFLOAT);

	else if ((bucket->spec_def & VOID_MASK)==VOID_MASK)
	    type = ty_build_basic(TYVOID);

	else if ((bucket->spec_def & LONGDOUBLE_MASK)==LONGDOUBLE_MASK)
	    type = ty_build_basic(TYLONGDOUBLE);

	else if((bucket->spec_def & UNSIGNEDLONGINT_MASK)==UNSIGNEDLONGINT_MASK)
	    type = ty_build_basic(TYUNSIGNEDLONGINT);

	else if ((bucket->spec_def & SIGNEDLONGINT_MASK)==SIGNEDLONGINT_MASK)
	    type = ty_build_basic(TYSIGNEDLONGINT);

	else if ((bucket->spec_def & SIGNEDSHORTINT_MASK)==SIGNEDSHORTINT_MASK)
	    type = ty_build_basic(TYSIGNEDSHORTINT);

	else if ((bucket->spec_def & UNSIGNEDSHORTINT_MASK)==
	    UNSIGNEDSHORTINT_MASK)
	    type = ty_build_basic(TYUNSIGNEDSHORTINT);

	else if ((bucket->spec_def & DOUBLE_MASK)==DOUBLE_MASK)
	    type = ty_build_basic(TYDOUBLE);

	else if ((bucket->spec_def & SIGNEDCHAR_MASK)==SIGNEDCHAR_MASK)
	    type = ty_build_basic(TYSIGNEDCHAR);

	else if ((bucket->spec_def & UNSIGNEDCHAR_MASK)==UNSIGNEDCHAR_MASK)
	    type = ty_build_basic(TYUNSIGNEDCHAR);

	else if ((bucket->spec_def & UNSIGNEDINT_MASK)==UNSIGNEDINT_MASK)
	    type = ty_build_basic(TYUNSIGNEDINT);

	else if ((bucket->spec_def & SIGNEDINT_MASK)==SIGNEDINT_MASK)
	    type = ty_build_basic(TYSIGNEDINT);

	else if ((bucket->spec_def & STRUCT_MASK)==STRUCT_MASK)
	    type = bucket->type;

	else if ((bucket->spec_def & UNION_MASK)==UNION_MASK)
	    type = bucket->type;

	else if ((bucket->spec_def & ENUM_MASK)==ENUM_MASK)
	    type = bucket->type;

	else if ((bucket->spec_def & TYPENAME_MASK)==TYPENAME_MASK)
	    type = bucket->type;

	else
	    bug("Illegal Declaration in build_base_type");

	return type;
    }

/*B*********************************************************************/
BOOLEAN chk_pointer(BOOLEAN err, BUCKET_PTR bucket)

	/* this routine does error checking for the pointer modifier
	  reports any errors and also returns true if an error
	  was found false other wise.

	  this is used to detect the following kind of errors:

		int * int x;
		int * unsigned x;

	  because remember this is legal:

		int * const x;
	 */

    {
	unsigned int mask;

	mask = BASE_MASK | SIZE_MASK | SIGN_MASK;

	if ((bucket != NULL) && (bucket->spec_def & mask))
		{
		error ("illegal pointer combination");
		err = TRUE;
		}

	return(err);
    }
