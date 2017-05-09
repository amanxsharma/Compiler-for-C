#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "defs.h"
#include "types.h"
#include "symtab.h"
#include "bucket.h"

FILE *errfp;		/* file to which message.c will write */

/* For debugging purposes only */
#ifdef YYDEBUG
extern int yydebug;
#endif

int main(int argc, char *argv[])
{
    int status, yyparse();
    BOOLEAN dump_flag = FALSE;

    if (argc==2 && (!strcmp(argv[1], "-d") || !strcmp(argv[1], "--dump")))
        dump_flag = TRUE;
    else if (argc > 1) {
        fprintf(stderr, "%s: bad command line argument(s)\n", argv[0]);
        exit(1);
    }
    
    errfp = stderr;
    ty_types_init();
    st_init_symtab();
    init_bucket_module();
    if (dump_flag) {
        st_establish_data_dump_func(stdr_dump);
        st_establish_tdata_dump_func(stdr_dump);
    }

#ifdef YYDEBUG
	yydebug = 1;		       /* DEBUG */
#endif
	status = yyparse();            /* Parse and translate the source */

        if (status==0 && dump_flag)    /* If parse was successful */
            st_dump();                 /* Dump the symbol table */

	return status;
}
