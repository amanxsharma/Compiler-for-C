/*
	BUCKET.H

	Routines in this module are designed to build base types through
	the processing of type specifiers.  The routine update_bucket does
	most of the work.  Basically, the routine is called repeatedly with
	different type specifiers that form a base type for a single
	declaration or series of declarations (ie. unsigned int a; or
	unsigned int a,b,c;).  As a side effect of this processing all
	semantic errors are checked for and reported.
*/

/* BUCKET_REC definition modified 05/27/89 */

#ifndef BUCKET_H
#define BUCKET_H

#include "types.h"

typedef struct bucket {
		      unsigned int spec_def;
		      TYPE  type;	/* type of struct/union/enum/typename */
		      BOOLEAN error_decl;
		      } BUCKET_REC, *BUCKET_PTR;


extern void init_bucket_module (void);
    /* 
	this routine initializes variables for use in the bucket module
    */

extern void print_bucket (BUCKET_PTR bucket);
    /* 
	   this routine prints whether a passed bucket record is nil
   */ 

extern BUCKET_PTR update_bucket (BUCKET_PTR bck,TYPE_SPECIFIER spec, TYPE type);
    /*

            This routine takes in a pointer to the type bucket and 
            also the specifier to be added to the record.  If there is not
            something already there it adds it to the mask else an error.
            The third parameter is usually NULL.  With enumerations, structs,
            and unions the third field has a type in it.  With typenames, the
            third parameter is an id to be looked up.

	    If the first parameter is NULL, then the routine will create a
	    new bucket record.
   */ 

extern STORAGE_CLASS get_class (BUCKET_PTR bucket);
    /*

	    This routine takes in the type info bucket and returns the storage
	    class of the mask
    */

extern TYPE_QUALIFIER get_qual (BUCKET_PTR bucket);
    /* 

	    This routine takes in the type info bucket and returns the type
	    qualifier(s) of the mask
    */

extern BOOLEAN is_error_decl (BUCKET_PTR bucket);
    /* 

	    This function returns TRUE if there was an error in the
	    declartion of the base type else returns FALSE
    */

extern TYPE build_base (BUCKET_PTR bucket);
    /* 

	    This routine takes in a bucket of type info and builds a base type
	    from the mask field
    */

extern BOOLEAN chk_pointer (BOOLEAN err, BUCKET_PTR bucket);
    /* 

	    This routine does error checking for the pointer modifier, reports
	    any errors, and also returns TRUE if an error was found, else FALSE
    */

#endif
