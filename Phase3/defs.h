#ifndef DEFS_H
#define DEFS_H

/****************************************************************/
/*								*/
/*	CSCE531 - Simplified "C++" Compiler	       		*/
/*								*/
/*	--defs.h--						*/
/*								*/
/*	This file contains general definitions for csce 531 	*/
/*	simplified C++ compiler.				*/
/*								*/
/*								*/
/*								*/
/****************************************************************/

typedef int BOOLEAN;

#define TRUE 1
#define FALSE 0

#ifndef NULL
#define NULL 0
#endif

typedef void * ST_ID;	/* symbol table identifier abstraction */

/* The current back-end include file */
#define BACKEND_HEADER_FILE "backend-x86.h"

/* What system are we on? */
#define SYS_LINUX

/* What is the source language? */
#undef PASCAL_LANG
/* Un-comment the next line for compiling Pascal */
/* #define PASCAL_LANG */

/* Remove comments to generate a parser trace: */
/* #define YYDEBUG 1 */

#endif
