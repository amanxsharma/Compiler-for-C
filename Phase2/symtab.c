/****************************************************************/
/*								*/
/*	PCC3 - "C/C++" Compiler					*/
/*								*/
/*	--symtab.c--						*/
/*								*/
/*	This file contains routines to manipulate               */
/*	the PCC3 symbol table.                           	*/
/*								*/
/*								*/
/*								*/
/****************************************************************/

#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "types.h"
#include "symtab.h"
#include "message.h"

/* #include <malloc.h> */


#define HT_LENGTH	997	/* hash table length */

#define ST_VAR 		0       /* bucket in "var" symbol table */
#define ST_TAG 		1       /* bucket in "tag" symbol table */


/* If L_warn_shadow is TRUE, st_install() checks for shadowed variables */
static int L_warn_shadow = FALSE;

typedef struct  stack_rec
{
    struct stack_rec *next_sr;  /* ptr next stack record */
    int    block_num;    	/* block rec defined in */
    struct stack_rec *block_sr; /* ptr next symbol in block*/
    ST_DR   data_rec;  	        /* ptr to data record */
    struct bucket_rec *home_br; /* ptr to home bucket */
    int typ_sr;  		/* ST_VAR = var ; ST_TAG = tag  */
} STACK_REC;

typedef struct  bucket_rec
{
    char *id;    		/* pointer to id string in string area */
    struct stack_rec *sr_top;   /* pointer to var stack */
    struct stack_rec *sr_tag;   /* pointer to tag stack */
    struct bucket_rec *next_br; /* pointer to next bucket */	
} ST_BUCKET_REC;


static int st_hash(char *);
static STACK_REC *alloc_stack_rec(void);
static char *alloc_str(int n);
static ST_BUCKET_REC *alloc_hash_rec(void);
static int st_hash(char *);


typedef STACK_REC  *BLOCK_ST;
typedef ST_BUCKET_REC *HASHTAB_REC_PTR;

/***********************************************************************/
/*	global internal symbol table variables                         */
/***********************************************************************/

static HASHTAB_REC_PTR hash_table[HT_LENGTH];
static BLOCK_ST block_stack[BS_DEPTH];    /* pointer to each block */ 
static int  cur_block = 0;   		  /* counter for current block level */
static user_func dump_func = NULL;	  /* free var stack data records */
static user_func dump_tag_func = NULL;    /* free tag stack data records */
static user_func free_func = NULL;	  /* free var stack data records */
static user_func free_tag_func = NULL;    /* free tag stack data records */
static STACK_REC *freesr = NULL;   	  /* ptr to free stack record space */


#ifdef PASCAL_LANG

/***********************************************************************/
/*	Predefined Pascal objects                                      */
/***********************************************************************/

/* Map predefined Pascal types to C types.
   This may be somewhat machine-specific */
static struct {
    char	*id;
    TYPETAG	tag;
} pascal_types[] = {
    {"Integer", TYSIGNEDLONGINT},
    {"Real", TYDOUBLE},
    {"Extended", TYLONGDOUBLE},
    {"Single", TYFLOAT},
    {"Word", TYSIGNEDSHORTINT},
    {"Byte", TYSIGNEDCHAR},
    {"Char", TYUNSIGNEDCHAR},
    {"Boolean", TYSIGNEDCHAR},
    {NULL, TYERROR}
};

#endif

/***********************************************************************/
/*                                                                     */
/*           Symbol Table Exported Routines                            */
/*                                                                     */
/***********************************************************************/

/*
**  Function: st_establish_data_dump_func
**
**  Purpose: copy f,user defined function,into static global variable dump_func
   	     used in st_dump and st_dump_block 
**
**  Notes:
*/

void st_establish_data_dump_func (user_func f)

{
    dump_func = f;
}



/*****************************************************************************/
/*
**  Function: st_establish_data_free_func
**
**  Purpose: copy f,user defined function, into static global variable
**  free_func used in st_exit_blcok 
**
**  Notes:
*/

void st_establish_data_free_func(user_func f)

{
    free_func = f;
}

/*****************************************************************************/
/*
**  Function: st_establish_tdata_dump_func
**
**  Purpose: copy f,user defined function, into static global variable 
	     dump_tag_func used in st_dump and st_dump_block
**
**  Notes:
*/

void st_establish_tdata_dump_func(user_func f)

{
    dump_tag_func = f;
}


/*****************************************************************************/
/*
**  Function: st_establish_tdata_free_func
**
**  Purpose: copy f,user defined function, into static global variable 
	     free_tag_func used in st_exit_blcok 
**
**  Notes:
*/


void st_establish_tdata_free_func(user_func f)

{
    free_tag_func = f;
}

/*****************************************************************************/
/*
**  Function: st_enter_block
**
**  Purpose: routine to open new block 
**
**  Notes:
*/

void st_enter_block(void)

{
    extern int cur_block;

    if (ST_DEBUG)
	msg("st_enter_block: cur_block will be %d.",cur_block+1);
    if ((cur_block + 1) < BS_DEPTH)
	cur_block = cur_block + 1;
    else
	fatal("MAX Stack DEPTH reached in \"st_enter_block\".\n");
}

/*****************************************************************************/
/*
**  Function: st_enter_id
**
**  Purpose: routine to enter id into symbol table
	   return : pointer to ST_ID in format of bucket record
	   look to see if is already there and return it
	   else allocate space and store string in string area 
**
**  Notes:
*/

ST_ID st_enter_id(char *id)
{
    int ht_ent;
    char *tmpid;
    ST_BUCKET_REC *new_br;

    ht_ent = st_hash(id);   /* get hash number */

    if (ST_DEBUG)
        msg("st_enter_id: hash table entry = %d.",ht_ent);

    new_br = hash_table[ht_ent];

    while (new_br != NULL)  /* check to see if id in symbol table */
    {
	tmpid = st_get_id_str((ST_ID) new_br);

	if (strcmp(tmpid,id))
	    new_br = new_br->next_br;   /* not equal */
	else  /* they are equal */
	    return(ST_ID)(new_br);
    }

    if ((new_br = alloc_hash_rec()) == NULL) /* not found so enter */
        fatal("Unable to allocate hash bucket space.\n"); 
    else
    {
	    /*  will store id-string in string pool area */
        new_br->id = st_save_string(id);

        if (ST_DEBUG)
            msg("st_enter_id: new id at = %d.",new_br->id);

        new_br->sr_top = NULL;
        new_br->sr_tag = NULL;
        new_br->next_br = hash_table[ht_ent];
        hash_table[ht_ent] = new_br;
    }

    return(ST_ID)(new_br);
}

/*****************************************************************************/
/*
**  Function: st_exit_block
**
**  Purpose: routine to free up stack records in exiting block
	   return space to free stack, call user defined function
	   to free data record space, and reset global variable cur_block 
**
**  Notes:
*/

void st_exit_block(void)
{  
    extern int cur_block;
    STACK_REC *curr;

#ifdef PASCAL_LANG
    if (cur_block <= 1)
	bug("Cannot exit from block 0 or 1 in \"st_exit_block\".\n");
#else
    if (cur_block == 0)
	bug("Cannot exit from block 0 in \"st_exit_block\".\n");
#endif
    else
    {	
	curr = block_stack[cur_block];

	while (curr != NULL)
        {
                /* remove from appropriate stack */
	    if (curr->typ_sr == ST_VAR)  /* belongs to var stack */
            {
		if(ST_DEBUG)
                {
		    msg ("st_exit_block: Deleting stack record-> %d.", curr);
		    msg ("st_exit_block: Stack Record id = %s.",
                         (st_get_id_str((ST_ID) curr->home_br)));
                }

		curr->home_br->sr_top = curr->next_sr;

		if (free_func != NULL)
		    (*free_func)(curr->data_rec);
            }
	    else if (curr->typ_sr == ST_TAG) /* belongs to tag stack */
            {
		if (ST_DEBUG)
                {
		    msg ("st_exit_block: Deleting stack record-> %d.", curr);
		    msg ("st_exit_block: Stack Record id = %s.",
                         (st_get_id_str((ST_ID) curr->home_br)));
                }

		curr->home_br->sr_tag = curr->next_sr;

		if (free_tag_func != NULL)
		    (*free_tag_func)(curr->data_rec);
            }
	    else
		bug("Invalild Stack Record in \"st_exit_block\".\n");

	    curr->next_sr = freesr;  /* add to free space stack */
	    freesr = curr;
	    curr = freesr->block_sr;
	    freesr->block_sr = NULL;
	    freesr->block_num  = 0;
	    freesr->home_br = NULL;
	    freesr->data_rec = NULL;
        }

	block_stack[cur_block] = NULL;
	cur_block = cur_block - 1;
    }
}

/*****************************************************************************/
/*
**  Function: st_get_cur_block
**
**  Purpose: return block number for current block 
**
**  Notes:
*/

int st_get_cur_block(void)

{
    return(cur_block);
}

/*****************************************************************************/
/*
**  Function: st_get_id_str
**
**  Purpose: parameter: idr ST_ID pointer to bucket record
	     return: id field of bucket record which is
	     pointer to actual string in string pool area 
**
**  Notes:
*/

char *st_get_id_str(ST_ID idr)

{
    ST_BUCKET_REC *tmpid;

    tmpid = (ST_BUCKET_REC *)idr;

    if (tmpid != NULL)
	return(tmpid->id);
    else
	return(NULL);
}

/**************************************************************************/
/*
**  Function: st_init_symtab
**
**  Purpose: initialize hash table or symbol table, and if Pascal,
**           install predefined Pascal identifiers
**
**  Notes:
*/

void st_init_symtab(void)   

{
    int i;
    ST_DR dr;

    /* Basic initialization of tables */
    for (i=0; i<HT_LENGTH; i++)
	hash_table[i] = NULL;

    for (i=0; i<BS_DEPTH; i++)
	block_stack[i] = NULL;

#ifdef PASCAL_LANG

    /**** It is safe to call symbol table functions below this line. ****/

    /* Install predefined Pascal things in block 0 */
    /* Predefined type names */
    for (i=0; pascal_types[i].id != NULL; i++)
    {
	dr = stdr_alloc();
	dr->tag = TYPENAME;
	dr->u.typename.type = ty_build_basic(pascal_types[i].tag);
	st_install(st_enter_id(pascal_types[i].id),dr);
    }

    /* Enter the user global block (block 1) */
    st_enter_block();

#endif
}

/*****************************************************************************/
/*
**  Function: st_install
**
**  Purpose: create stack record, determine current block, link into block 
	     stack list by inserting in front, and push onto appropriate id 
	     stack, establish back link to home id bucket      
	     return true(1) if installed else false(0) if already installed 
**
**  Notes:
*/

BOOLEAN st_install(ST_ID id, ST_DR data)
{
    STACK_REC *new_sr;
    ST_BUCKET_REC *tmp_br;
    ST_DR chckptr;
    int chck;

    if (id == NULL)
	bug("null id passed to \"st_install\"");

        /* Added the shadow check capability -- AL, 8-90 */
    if ((chckptr = st_lookup(id,&chck)) != NULL)
	if (chck == cur_block)
	    return FALSE;
	else if (L_warn_shadow)
        {
	    warning ("symbol \"%s\" shadows previous declaration",
                     st_get_id_str(id));
        }

        /* allocate stack record space from pool */
    if ((new_sr = alloc_stack_rec()) == NULL)  
	fatal("Unable to allocate stack record space.\n");
    else
    {
	tmp_br = (ST_BUCKET_REC *)id;
	new_sr->next_sr = tmp_br->sr_top;
	new_sr->block_num = cur_block;
	new_sr->block_sr = block_stack[cur_block];
	new_sr->typ_sr = ST_VAR;
	new_sr->data_rec = data;
	new_sr->home_br = (ST_BUCKET_REC *)id;
	block_stack[cur_block] = new_sr;
	tmp_br->sr_top = new_sr;

	if (ST_DEBUG)
        {
	    msg("st_install: new br = %x", new_sr);
	    msg("st_install: new->next_sr = %x", new_sr->next_sr);
	    msg("st_install: new->block# = %d", new_sr->block_num);
	    msg("st_install: new->bl_sr = %x", new_sr->block_sr);
	    msg("st_install: new->h_br = %x", new_sr->home_br);
	    msg("st_install: new->data = %x", new_sr->data_rec);

	    if (dump_func != NULL)
		(*dump_func)(new_sr->data_rec);
        }

	return TRUE;
    }

    bug ("How'd control get here (%d %s)?", __LINE__, __FILE__);
}

/*****************************************************************************/
/*
**  Function: st_lookup
**
**  Purpose: parameters : id is ST_ID to home bucket record
	     block is output parameter ptr. to block where item found
             returns : if found, returns pointer to data record else null
**
**  Notes:
*/

ST_DR st_lookup(ST_ID id, int *block)
{ 
    ST_BUCKET_REC *tmpbr;

    if ((tmpbr = (ST_BUCKET_REC *)id) == NULL)
        return NULL;
    
    if (tmpbr->sr_top == NULL)
        return NULL;

    *block = tmpbr->sr_top->block_num;
    return(tmpbr->sr_top->data_rec);
}

/*****************************************************************************/
/*
**  Function: st_replace
**
**  Purpose: parameters : id is ST_ID to home bucket record
	     data is a user data record 
             returns :  if data record already exists, replace it with input
	     and return TRUE; else return FALSE.
**
**  Notes:
*/

BOOLEAN st_replace(ST_ID id, ST_DR data)
{ 
    ST_BUCKET_REC *tmpbr;

    if ((tmpbr = (ST_BUCKET_REC *)id) == NULL)
        return FALSE;

    if (tmpbr->sr_top == NULL)
        return FALSE;

    tmpbr->sr_top->data_rec = data;
    return TRUE;
}


/***************************************************************************/
/*
**  Function: st_lookup_id
**
**  Purpose: this routine takes a pointer to a character string/identifier 
             and returns the bucket record pointer as a ST_ID pointer
             if the id has been entered; else returns NULL
**
**  Notes:
*/

ST_ID st_lookup_id(char *itm)
{	
    ST_BUCKET_REC  *tmp_br;
    int ht_ent;
    char *tmpid;
	
    ht_ent = st_hash(itm);   /*  get hash number */

    tmp_br = hash_table[ht_ent];

    while (tmp_br != NULL)  /* find id in symbol table */
    {
	tmpid = st_get_id_str((ST_ID) tmp_br);

	if (strcmp(tmpid,itm))
	    tmp_br = tmp_br->next_br;   /* not equal */
	else  /* they are equal */
	    return (ST_ID)(tmp_br);
    }

    return NULL;
}

/*****************************************************************************/
/*
**  Function: st_save_string
**
**  Purpose:
**
**  Notes:
*/

char *st_save_string(const char *str)

{
    char *pstr;
	
    if ((pstr = alloc_str(strlen(str))) == NULL)	
	fatal("Ran out of mem in \"st_save_string\".\n");
    else
	strcpy(pstr,str);

    return(pstr);
}

/*****************************************************************************/
/*
**  Function: st_tag_install
**
**  Purpose: create stack record, determine current block,
   	     link into block stack list by inserting in front, 
   	     and push onto appropriate id stack,
   	     establish back link to home id bucket
   	     return true(1) if installed esle false(0) if already installed 
**
**  Notes:
*/

BOOLEAN st_tag_install(ST_ID id, ST_DR data)
{
    STACK_REC *new_sr;
    ST_BUCKET_REC *tmp_br;
    int   chck;

    if ((st_tag_lookup(id,&chck) != NULL) && (chck == cur_block))
	return FALSE;

        /* allocate stack record space */
    if ((new_sr = alloc_stack_rec()) == NULL) 
	fatal("Ran out of mem in \"st_tag_install\".\n");
    else
    {
	tmp_br = (ST_BUCKET_REC *)id;
	new_sr->next_sr = tmp_br->sr_tag;
	new_sr->block_num = cur_block;
	new_sr->typ_sr = ST_TAG;
	new_sr->block_sr = block_stack[cur_block];
	new_sr->data_rec = data;
	new_sr->home_br = (ST_BUCKET_REC *)id;
	block_stack[cur_block] = new_sr;
	tmp_br->sr_tag = new_sr;

	if (ST_DEBUG)
        {
	    msg("st_tinstall: new->next_sr = %d", new_sr->next_sr);
	    msg("st_tinstall: new->block# = %d", new_sr->block_num);
	    msg("st_tinstall: new->bl_sr = %d", new_sr->block_sr);
	    msg("st_tinstall: new->h_br = %d", new_sr->home_br);
	    msg("st_tinstall: new->data = %d", new_sr->data_rec);

	    if (dump_tag_func != NULL)
		(*dump_tag_func)(new_sr->data_rec);
        }

	return TRUE;
    }

    bug ("How'd control get here (%d %s)?", __LINE__, __FILE__);
}

/*****************************************************************************/
/*
**  Function: st_tag_lookup
**
**  Purpose: parameters : id is ST_ID
	     block is output parameter ptr. to blcok where item found
             returns : if found, returns pointer to data record else null
**
**  Notes:
*/

ST_DR st_tag_lookup(ST_ID id, int *block)
{ 
    ST_BUCKET_REC *tmpbr;

    if ((tmpbr = (ST_BUCKET_REC *)id) == NULL)
        return NULL;

    if (tmpbr->sr_tag == NULL)
        return NULL;

    *block = tmpbr->sr_tag->block_num;
    return(tmpbr->sr_tag->data_rec);
}

/*****************************************************************************/
/*
**  Function: st_tag_replace
**
**  Purpose: parameters : id is ST_ID
	     data is a user defined data record
             returns : if data record already exists for id, replace it and
	     return TRUE; else return FALSE.
**
**  Notes:
*/

BOOLEAN st_tag_replace(ST_ID id, ST_DR data)

{ 
    ST_BUCKET_REC *tmpbr;

    if ((tmpbr = (ST_BUCKET_REC *)id) == NULL)
        return FALSE;

    if (tmpbr->sr_tag == NULL)
        return FALSE;

    tmpbr->sr_tag->data_rec = data;
    return TRUE;
}

/***************************************************************************/
/*                                                                         */
/*	dump routine section					           */
/*                                                                         */
/***************************************************************************/

/*
**  Function: st_dump
**
**  Purpose: dump of each id in hash table and for each their two stacks 
**
**  Notes:
*/

void st_dump (void)
{
    int i;
    ST_BUCKET_REC *gothru;
    STACK_REC *gosr;

    for(i=0; i<HT_LENGTH; i++)  /* go through hash table */
    {
        for (gothru = hash_table[i]; gothru != NULL; gothru = gothru->next_br)
        {
                /* print id string and then for each id
                   print var stack records and tag stack records
                   and finally next bucket record */

            msg("\n\n        Dump: Id  = %s",st_get_id_str((ST_ID) gothru));

            if (gothru->sr_top != NULL)
            {
                gosr = gothru->sr_top;
                msg("\n        Dump: VAR Stack. ");

                while (gosr != NULL)
                {
                    msg("            Dump: block num =%d", gosr->block_num);

                    if (dump_func != NULL)
                        (*dump_func)(gosr->data_rec);
                    else
                        msg (" Dump: No user defined data dump functions.");

                    gosr = gosr->next_sr;
                }
            }

            if (gothru->sr_tag != NULL)
            {
                gosr = gothru->sr_tag;
                msg("        Dump: TAG Stack. ");

                while (gosr != NULL)
                {
                    msg("            Dump: block num =%d", gosr->block_num);

                    if (dump_tag_func != NULL)
                        (*dump_tag_func)(gosr->data_rec);
                    else
                        msg (" Dump: No user defined data dump functions.");

                    gosr = gosr->next_sr;
                }
            }
        }
    }
}

/**************************************************************************/

/*
**  Function: st_dump_block
**
**  Purpose: routine to dump a requested block's stack records
	     parameter : block number to dump 
**
**  Notes:
*/

void st_dump_block(int num)
{
    STACK_REC *gosr;

    msg("\n	DUMP OF BLOCK NUMBER = %d",num);

    if (num < 0 || num > cur_block)
	msg("bad block number: current block is %d", cur_block);

    if ((gosr = block_stack[num]) != NULL) 
    {
	while (gosr != NULL)
        { 
	    msg("\n    Dump: block num =%d",gosr->block_num);
	    msg("    Dump: home_bucket = %d",gosr->home_br);
	    msg("    Dump: next record in stack = %d",gosr->next_sr);
	    msg("    Dump: next record in block =%d",gosr->block_sr);
	    msg("    Dump: Id = %s",(st_get_id_str((ST_ID) gosr->home_br)));

	    if (gosr->typ_sr == ST_VAR) 
            {
	        if (dump_func != NULL)
		    (*dump_func)(gosr->data_rec);
		else
		    msg ("      Dump: No user defined data dump functions");
            } 
	    else if (gosr->typ_sr == ST_TAG) 
            {
		if (dump_tag_func != NULL)
		    (*dump_tag_func)(gosr->data_rec);
		else
		    msg ("      Dump: No user defined data dump functions");
            } 
	    else
		bug("Stack Record with no id-tag in \"st_dump_block\"\n");

	    gosr = gosr->block_sr;
        }
    }
    else
	msg("	DUMP : Block Scope Empty ");
}

/*****************************************************************************/
/*                                                                           */
/*     st_dump_st_id :              					     */
/*		input parameter: ST_ID                                       */
/*		print out the string identifier for this record              */
/*                                                                           */
/*****************************************************************************/

void st_dump_st_id (ST_ID idptr)

{
    char *idstr;

    if ((idstr = st_get_id_str(idptr)) !=  NULL)
	msg("\"%s\" ",idstr);
    else
	msg("<null>");
}

/***************************************************************************/
/*                                                                         */
/*           All the following routines will                               */
/*           be hidden from the user of the symbol table routines          */
/*                                                                         */
/***************************************************************************/

/***********************************************************************/
/*                                                                     */
/*	 Global Definitions for Three Storage Pools                    */
/*                                                                     */
/***********************************************************************/


#define BRPOOL     	200
#define SRPOOL		200
#define	STRPOOL		200

static char  *strp = NULL;    
static int strpool_num = STRPOOL;

static ST_BUCKET_REC brbuff[BRPOOL]; /* pool area and ptr for bucket records */
static ST_BUCKET_REC *brp = brbuff;
static int bpool_num = 0;	     /* count of used records */

static STACK_REC srbuff[SRPOOL];/* need pool area & pointer for stack records */
static STACK_REC *srp = srbuff;
static int spool_num = 0;            /* count of used records */

/***********************************************************************/
/***********************************************************************/

/*
**  Function: alloc_str
**
**  Purpose: this routine returns a pointer to where the character
	     string was placed in the string pool area 
**
**  Notes:
*/

static char *alloc_str(int n)
{
    if (strpool_num + n + 1 <=  STRPOOL)  /* fits */
	strpool_num = strpool_num + n + 1;
    else   /* not enough room */
    {
	if ((strp = malloc(STRPOOL)) != NULL) 
	    strpool_num = n + 1;
	else
	    fatal("Ran out of mem in \"alloc_str\"\n");
    }

    strp = strp + n + 1;
    return (strp - n - 1);
}

/************************************************************************/
/************************************************************************/
/*
**  Function: allod_hash_rec
**
**  Purpose: this routine returns a pointer to a bucket record
	     from the pool of available records 
**
**  Notes:
*/

static ST_BUCKET_REC *alloc_hash_rec(void)
{
    ST_BUCKET_REC *retbr;

    if (bpool_num < BRPOOL)
    {
        retbr = brp;
        brp++;
        bpool_num++;
    }
    else
    {
        if (brp = (ST_BUCKET_REC *) calloc(BRPOOL,sizeof(struct bucket_rec)))
        {
	    bpool_num = 1;
	    retbr = brp;
	    brp++;
        }
	else
	    fatal("Ran out of mem in \"alloc_hash\"\n");
    }

    return retbr;
}

/***********************************************************************/
/*
**  Function: alloc_stack_rec
**
**  Purpose: routine returns a pointer to a stack record
             from the pool of available records 
**
**  Notes:
*/

static STACK_REC *alloc_stack_rec(void)
{
    STACK_REC *retsr;

    if (freesr != NULL)
    {
        if (ST_DEBUG)
            msg("alloc_stack : alloc from free stack.");

        retsr = freesr;
        freesr = freesr->next_sr;
        retsr->next_sr = NULL;   /* pjh: 07/18/89 */
        return(retsr);
    }
    else
    {
        if (spool_num < SRPOOL)
        {
            retsr = srp;
            srp++;
            spool_num++;
        }
        else
        {
            if (srp = (STACK_REC *) calloc(SRPOOL,sizeof(struct stack_rec)))
            {
                spool_num = 1;
                retsr = srp;
                srp++;
            }
            else
                fatal("Ran out of mem in \"alloc_stack\"\n");
        }

        retsr->next_sr = NULL;  /* pjh: 07/18/89 */
        return retsr;
    }
}

/************************************************************************/
/************************************************************************/

/*
**  Function: st_hash
**
**  Purpose: st_hash -
     	     parameter : string id
	     purpose : to calculate a hash table lookup value
	     return  : integer entry to hash table 
**
**  Notes:
*/

static int st_hash (char *id)
{
    int sum = 0;

    for (; *id != '\0'; id++)
	sum = (37 * sum + *id) % HT_LENGTH;

    return sum;
} 

void st_warn_shadow (int flag)
{
    L_warn_shadow = flag;
}

