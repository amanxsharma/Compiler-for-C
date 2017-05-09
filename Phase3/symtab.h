/* SYMTAB.H
*/

#ifndef SYMTAB_H
#define SYMTAB_H

#include "types.h"

#define ST_DEBUG 0		/* enable/disable symbol table debugging */
#define BS_DEPTH 11		/* block stack depth; maximum scope nesting */

typedef void (*user_func)();

/****************************************************************************/
/* 
	definitions for symbol table data record
*/

typedef enum {ECONST, GDECL, LDECL, PDECL, FDECL, TAG, TYPENAME} STDR_TAG;

typedef struct st_dr {
    STDR_TAG tag;
    union {
        struct {
            TYPE type;
            int val;
        } econst;
        struct {
            TYPE type;
            STORAGE_CLASS sc;
            int binding;
            int regno;
	    BOOLEAN is_ref;
	    union {   /* Pascal only */
		int offset;
		char *global_func_name;
	    } v;
            BOOLEAN err;
        } decl;
        struct {	/* stag: Struct/Union/Enum tag */
            TYPE type;
        } stag;
	struct {
	    TYPE type;
	} typename;
    } u;
} ST_DATA_REC, *ST_DR;

ST_DR stdr_alloc(void);
/*
  Takes no parameters. Returns a new symbol table data record.
*/

void stdr_free(ST_DR stdr);
/*
  Frees a symbol table data record.
*/

void stdr_dump(ST_DR stdr);
/*
  Displays a symbol table data record using msg/msgn from message.c
*/

/****************************************************************************/
/* 
	definitions to support symtab.c

	symtab.c implements two block structured symbol tables: one
	for struct/union/enum tags and one for other things: variables,
	functions, typedef names, etc.

	the symtab.c also provides a facility for storing strings in
	"safe" spots: see st_save_string().
*/

/*
ST_DR: name given to the type of the symbol table data records.
 
Symbol table does not need to know details of the data records.  Internally
the symbol table will treat ST_DR as an OPAQUE type. The user simply should
declare ST_DR to be a pointer to some record. The declaration of ST_DR
should be in this file and above this point.

*/

/* enables symbol table debugging */
#ifndef ST_DEBUG
#define ST_DEBUG 1
#endif


void  st_init_symtab(void);
/*
  No parameters: must be called once before any other symbol table
  routine is invoked. Block for globals is entered automatically.
*/

void   st_warn_shadow (int);
/*
  If TRUE is passed as the parameter to this function, st_install()
  will first see if the identifier being installed has already
  been installed at a lower-numbered block.  If it has, a warning
  is issued.

  If called with FALSE (or not called at all), no warning is issued.
*/

ST_ID  st_enter_id(char *id);
/*
  This routine "enrolls" an identifier into the symbol table. Only
  after an identifier is enrolled can data be associated with that
  identifier. The symbol table makes no assumptions about identifiers
  other than they are null terminated strings. The ST_ID returned
  is the "enrollment papers" for the identifier and are presented
  to the symbol table when future actions are requested for the
  identifier. Once an id is enrolled, it stays enrolled, even if the
  block in which it was enrolled is exited.  (PROJ1)
*/

char  *st_get_id_str(ST_ID id);
/*
  Given the "enrollment papers" of an identifier, returns
  the identifier string.  (PROJ1)
*/

void  st_enter_block(void);
/*
  No parameters: informs the symbol table that a new block
  has been entered. Increments the "current block" number.
*/

void  st_exit_block(void);
/*
  No parameters: informs the symbol table that a block has
  been exited. Decrements the "current block" number. All data
  installed in the exited block is removed.
*/

int  st_get_cur_block(void);
/*
  No parameters: returns the block number of the current block.
  For C/C++, globals are in block zero.  For Pascal, globals are in block
  one; predefined objects are in block zero.
  (PROJ1)
*/

BOOLEAN st_install(ST_ID id, ST_DR data);
BOOLEAN st_tag_install(ST_ID id, ST_DR data);
/*
  Installs a piece of data with the enrolled id. Returns FALSE
  if a piece of data is already installed with the given id; TRUE
  otherwise. st_tag_install installs data in the "tag" symbol table
  and st_install installs data in the "normal" symbol table. Data
  is installed in the current block.  (PROJ1)
*/

extern ST_DR  st_lookup(ST_ID id, int *block);
extern ST_DR  st_tag_lookup(ST_ID id, int *block);
/*
  Looks up and returns the data record associated with the enrolled
  identifier passed in as first parameter. The second parameter is
  an output parameter: the block number of the block in which the
  data was found. A search of blocks from the current back to the
  global is performed. The first occurence found is returned.
  st_lookup looks in the "normal" symbol table. st_tag_lookup looks
  in the "tag" symbol table. If no data is found, NULL is returned.
*/

extern BOOLEAN st_replace(ST_ID id, ST_DR data);
extern BOOLEAN st_tag_replace();
/*
  Replaces a piece of data for the enrolled id. Returns FALSE
  if no data is presently installed with the given id; TRUE
  otherwise. st_tag_replace replaces data in the "tag" symbol table
  and st_replace replaces data in the "normal" symbol table.
*/

extern ST_ID  st_lookup_id(char *id);
/*
  Checks to see if an identifier has been previously enrolled in the
  symbol table. If so, the "enrollment papers" are returned; if not,
  NULL is returned. This is probably of limited use since IDs are
  automatically enrolled by the scanner and st_enter_id works fine
  (returns enrollment papers) if id is already enrolled. This routine
  would only be necessary if you need to know if an id is in the
  symbol table or not. Note, however, this does not tell if there
  is data with the id: it simply checks whether the ID is enrolled
  or not. By the way, once an id is enrolled, it stays enrolled, even
  if the block in which it was enrolled is exited.
*/

extern void st_establish_data_free_func(user_func f);
extern void st_establish_tdata_free_func(user_func f);
/*
  Takes one parameter, a pointer to a function returning void.
  This tells the symbol table routines to use the parameter to
  invoke a function to "free" the data records when a block is
  exited. More exactly: when a block is exited, the function
  pointed to by the parameter will be called once for each data
  record installed in the block being exited. The data records
  will be passed as the only parameter to the "established" function.
  If no function has been "established" prior to a block being
  exited, then no "free" function is called.

  st_establish_data_free_func is pertinent to the "normal" symbol
  table. st_establish_tdata_free_func is pertinent to the "tag"
  symbol table.
*/

extern void st_dump(void);
/*
  Takes no parameters. This is a debugging routine that takes a
  "snapshot" (using msg/msgn in message.c) of the symbol table.
  It displays the complete contents of both the "normal" and "tag"
  symbol tables.
*/

extern void st_dump_block(int num);
/*
  Takes a snapshot of one particluar block in the symbol table.
*/

extern void   st_establish_data_dump_func(user_func f);
extern void   st_establish_tdata_dump_func(user_func f);
/*
  Both take a pointer to a function that should be invoked to display
  data records when a dump routine is invoked.
*/

extern void st_dump_st_id(ST_ID id);
/*
  Displays the id string of the given "enrolled" id.
*/

extern char  *st_save_string(const char *str);
/*
  Allocates space in a safe area and copies the given string to
  the newly allocated space. Returns a pointer to the newly
  allocated space.
*/

#endif
