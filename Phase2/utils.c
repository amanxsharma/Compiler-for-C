
/****************************************************************/
/*								*/
/*	CSCE531 - "Pascal" and "C" Compilers			*/
/*								*/
/*	--utils.c--						*/
/*								*/
/*	This file contains utility routines for the CSCE531	*/
/*	Pascal and C compilers.					*/
/*								*/
/*								*/
/*								*/
/****************************************************************/

#include <stdarg.h>
#include <stdlib.h>
#include "defs.h"
#include "types.h"
#include "symtab.h"
#include "message.h"
/* defined in defs.h */
#include BACKEND_HEADER_FILE

/****************************************************************/
/*
	interface to scanner to get line numbers for error messages
*/

int sc_line()
{
	extern int yylineno;

	return yylineno;
}

/****************************************************************/
/*
	support for parameter lists
*/
PARAM_LIST plist_alloc()
{
	return (PARAM_LIST) malloc(sizeof(PARAM));
}

void plist_free(PARAM_LIST plist)
{
    free(plist);
}

/****************************************************************/
/*
	support for symbol table data records
*/

void stdr_free(ST_DR stdr)
{
	free(stdr);
}

ST_DR stdr_alloc()
{
	return (ST_DR) malloc(sizeof(ST_DATA_REC));
}

void stdr_dump(ST_DR stdr)
{
	extern void print_boolean(BOOLEAN);

	if (stdr == NULL)
	{
		msg("<null>");
		return;
	}
	switch(stdr->tag) {
	case ECONST:	msg("	ECONST");
			break;
	case GDECL:	msg("	GDECL");
			break;
	case LDECL:	msg("	LDECL");
			break;
	case PDECL:	msg("	PDECL");
			break;
	case FDECL:	msg("	FDECL");
			break;
	case TAG:	msg("	TAG");
			break;
	case TYPENAME:	msg("	TYPENAME");
			break;
	default:
		bug("illegal tag in \"stdr_dump\""); 
	}
	switch(stdr->tag) {
	case ECONST:	msgn("		type = ");
			ty_print_type(stdr->u.econst.type);
			msg("");
			msg("		value = %d",stdr->u.econst.val);
			break;
	case GDECL:
	case LDECL:
	case PDECL:
	case FDECL:	msgn("		type = ");
			ty_print_type(stdr->u.decl.type);
			msg("");
			msgn("		storage class = ");
			ty_print_class(stdr->u.decl.sc);
			msg("");
			msgn("		reference parameter = ");
			print_boolean(stdr->u.decl.is_ref);
			msg("");
#ifdef PASCAL_LANG
			if (stdr->tag == LDECL || stdr->tag == PDECL)
			    msg("		offset = %d",
				stdr->u.decl.v.offset);
			else if (stdr->tag == FDECL)
			    msg("		global function name = \"%s\"",
				stdr->u.decl.v.global_func_name);
#endif
			msgn("		error = ");
			print_boolean(stdr->u.decl.err);
			msg("");
			break;
	case TAG:	msgn("		type = ");
			ty_print_type(stdr->u.stag.type);
			msg("");
			break;
	case TYPENAME:	msgn("		type = ");
			ty_print_type(stdr->u.typename.type);
			msg("");
			break;
	}
}


/****************************************************************/
/*
	support for printing "typedef" types
*/

void print_boolean(BOOLEAN b)
{
	if (b)
		msgn("TRUE");

	else
		msgn("FALSE");
}


/****************************************************************/
/*
	backend support for allocating global variables

	1st arg: the type (TYPETAG)
	2nd arg: the initialization value (dependent on type)
*/

void b_alloc_gdata ( TYPETAG tag, ... )
{
    va_list ap;
    va_start(ap, tag);
    switch (tag) {
    case TYSIGNEDCHAR:
    case TYUNSIGNEDCHAR:
	b_alloc_char(va_arg(ap, int));
	break;
    case TYSIGNEDSHORTINT:
    case TYUNSIGNEDSHORTINT:
	b_alloc_short(va_arg(ap, int));
	break;
    case TYSIGNEDINT:
    case TYUNSIGNEDINT:
	b_alloc_int(va_arg(ap, int));
	break;
    case TYSIGNEDLONGINT:
    case TYUNSIGNEDLONGINT:
	b_alloc_long(va_arg(ap, long));
	break;
    case TYPTR:
	b_alloc_ptr(va_arg(ap, char *));
	break;
    case TYFLOAT:
	b_alloc_float(va_arg(ap, double));
	break;
    case TYDOUBLE:
    case TYLONGDOUBLE:
	b_alloc_double(va_arg(ap, double));
	break;
    default:
	bug("alloc_gdata: cannot allocate global data of this type");
    }
    va_end(ap);
}



/****************************************************************/
/* Note: We cannot just use sizeof() here, because the runtime  */
/*       memory model may differ from the compile-time memory   */
/*       model.  Currently, we assume the 32-bit x86 model.     */
/****************************************************************/
unsigned int get_size_basic(TYPETAG tag)
{
    switch (tag) {
    case TYSIGNEDCHAR:
    case TYUNSIGNEDCHAR:
	return 1;
    case TYSIGNEDSHORTINT:
    case TYUNSIGNEDSHORTINT:
	return 2;
    case TYSIGNEDINT:
    case TYUNSIGNEDINT:
    case TYENUM:
	return 4;
    case TYSIGNEDLONGINT:
    case TYUNSIGNEDLONGINT:
	return 4;
    case TYPTR:
	return 4;     /* this is for 32-bit, which differs from 64-bit */
    case TYFLOAT:
	return 4;
    case TYDOUBLE:
    case TYLONGDOUBLE:
	return 8;
    case TYVOID:
	return sizeof(void);
    case TYFUNC:
        bug("get_size_basic: TYFUNC is not a data type");
    case TYERROR:
        bug("get_size_basic: TYERROR is not a data type");
    default:
	bug("get_size_basic: nonbasic or unknown type: %d", tag);
    }
}
