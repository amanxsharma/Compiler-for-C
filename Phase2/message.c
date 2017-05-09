
/****************************************************************/
/*								*/
/*	CSCE531 - "Pascal" and "C" Compilers			*/
/*								*/
/*	--message.c--						*/
/*								*/
/*	This File Contains routines that support the		*/
/*	"Message Module".					*/
/*								*/
/*								*/
/*								*/
/****************************************************************/



#include <stdio.h>
#include <stdlib.h>
#include "message.h"

/* Counter to keep track of the number of compiler errors */
int compiler_errors = 0;


/* Counter to keep track of the number of compiler warnings */
int compiler_warnings = 0;


/* Counter to keep track of the number of informal compiler messages */
int compiler_messages = 0;


/* error file pointer: assumed to be declared and initialized elsewhere */
extern FILE *errfp;


void msgn( char *format, ... )
	/* writes a message passed in to specified file with
		no newline
	*/
{
	va_list ap;
	va_start(ap, format);
	vfprintf(errfp, format, ap);
	compiler_messages++;
	va_end(ap);
}


void msg( char *format, ... )
	/* writes a message passed in to specified file with
		 newline.
	*/
{
	va_list ap;
	va_start(ap, format);
	vfprintf(errfp, format, ap);
	putc('\n',errfp);
	compiler_messages++;
	va_end(ap);
}


void message( char *format, ... )
	/* writes a message passed in to specified file with
		 line number and newline.
	*/
{
	va_list ap;
	va_start(ap, format);
	fprintf(errfp,"line %d: ",sc_line());
	vfprintf(errfp, format, ap);
	putc('\n',errfp);
	compiler_messages++;
	va_end(ap);
}


static void vmessage( char *format, va_list ap )
	/* writes a message passed in to specified file with
		 line number and newline.  Assumes prestarted va_list
		 and does not end the va_list
	*/
{
	fprintf(errfp,"line %d: ",sc_line());
	vfprintf(errfp, format, ap);
	putc('\n',errfp);
	compiler_messages++;
}


void warning ( char *format, ... )
	/* this routine issues a formal warning to user including
	  message passed in to specified file.
	 */
{
	va_list ap;
	va_start(ap, format);
	fprintf(errfp,"line %d: ",sc_line());
	fprintf(errfp, "WARNING -- ");
	vfprintf(errfp, format, ap);
	putc('\n',errfp);
	compiler_warnings++;
	va_end(ap);
}


void error( char *format, ... )
	/* this routine issues a compiler error in users source including
	  message passed in to specified file.
	 */
{
	va_list ap;
	va_start(ap, format);
	fprintf(errfp,"line %d: ",sc_line());
	fprintf(errfp, "ERROR -- ");
	vfprintf(errfp, format, ap);
	putc('\n',errfp);
	compiler_errors++;
	va_end(ap);
}


void fatal( char *format, ... )
	/* this routine writes the message passed in to a specified
	  file with the tag "fatal" meaning an internal data structure
	  of the compiler has overflowed.
	 */
{
	va_list ap;
	va_start(ap, format);
	fputs("[fatal error] ", errfp);
	vmessage(format, ap);
	va_end(ap);
	exit(1);
}


void bug( char *format, ... )
	/* this routine writes the message passed in to a specified
	  file with the tag "bug" meaning a problem with the internals
	  of the compiler.
	 */
{
	va_list ap;
	va_start(ap, format);
	fputs("BUG: ", errfp);
	vmessage(format, ap);
	va_end(ap);
	exit(1);
}

/*********************************************************************/
