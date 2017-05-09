/****************************************************************************/
/* 
	definitions to support message.c
*/

#ifndef MESSAGE_H
#define MESSAGE_H

#include <stdarg.h>
#include "defs.h"

/* Global Count of Compiler Errors */
extern int compiler_errors;

/* Global Count of Compiler Warnings */
extern int compiler_warnings;

/* Global Count of Compiler Messages */
extern int compiler_messages;


/****************************************************************/
/* Available Message routines */

/* Note: All message routine arguments take a "printf" like
	format.
		ex. msg("%d %d \n",x,y)

	NOTE: the routines all print using the FILE variable named
	      "errfp". This variable is assumed to be declared and
	      initialized outside message.c.

	NOTE: the message routines obtain the source file line number
	      by calling the scanner routine sc_line.
 */


/*********************************************************************/
extern void msgn( char *format, ... );
    /*
	this routine writes the message passed in to a specified 
		file with no newline added.
     */

/*********************************************************************/
extern void msg( char *format, ... );
    /*
	this routine writes the message passed in to a specified 
		file with newline added.
     */

/*********************************************************************/
extern void message( char *format, ... );
    /*
	this routine writes the message passed in to a specified 
		file with line number of source and newline added.
     */

/*********************************************************************/
extern void warning( char *format, ... );
    /*
	this routine issues a formal warning to user including
	  message passed in and line number to specified file.
     */

/*********************************************************************/
extern void error( char *format, ... );
    /*
	this routine issues a formal error to user including
	  message passed in and line number to specified file.
     */

/*********************************************************************/
extern void fatal( char *format, ... );
    /*
	 this routine writes the message passed in to a specified
	  file with the tag "fatal" meaning an internal data structure
	  of the compiler has overflowed.
     */

/*********************************************************************/
extern void bug( char *format, ... );
    /*
	 this routine writes the message passed in to a specified
	  file with the tag "bug" meaning a problem with the internals
	  of the compiler.
     */

/****************************************************************************/
/* 
	definitions to support utility routines
*/

/**************************************************************/
/* returns the current source file line number                             */

extern int sc_line();

/**************************************************************/
/* uses msg/msgn to display its single parameter of type BOOLEAN	   */

extern void print_boolean();

#endif
