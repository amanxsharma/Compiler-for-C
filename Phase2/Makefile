#
# pcc3 makefile
#
#

#
# use which C compiler?
#
CC = gcc

#
# use which parser generator?
#
YACC = bison

#
# Yacc/Bison flags
#
YFLAGS = -d -y

#
# use which scanner generator?
#
LEX = flex

#
# compile everything for debugger?
#
#yes
CFLAGS = -g
#no
#CFLAGS =

#
# which back end?
#
BACKEND = backend-x86

PCC3H	= defs.h types.h bucket.h tree.h encode.h expr.h symtab.h $(BACKEND).h

PCC3OBJ = main.o message.o symtab.o types.o bucket.o tree.o encode.o expr.o utils.o gram.o scan.o $(BACKEND).o

# pcc3 rules
#
pcc3     : $(PCC3OBJ)
	$(CC) $(CFLAGS) $(PCC3OBJ) -o pcc3

# dependencies for compiler modules
expr.o: expr.h expr.c

encode.o: encode.h encode.c

tree.o: tree.h tree.c 

main.o: main.c defs.h types.h symtab.h

types.o: types.c types.h symtab.h message.h

bucket.o: bucket.c bucket.h

symtab.o: symtab.c types.h symtab.h message.h

$(BACKEND).o: $(BACKEND).c $(BACKEND).h message.h defs.h

message.o: message.c message.h defs.h

utils.o: utils.c symtab.h message.h defs.h $(BACKEND).h

gram.o : gram.y $(PCC3H)
	$(YACC) $(YFLAGS) gram.y
	$(CC) $(CFLAGS) -c y.tab.c
	mv y.tab.o gram.o

scan.o : scan.l gram.o $(PCC3H)
	$(LEX) scan.l
	$(CC) $(CFLAGS) -c lex.yy.c
	rm lex.yy.c
	mv lex.yy.o scan.o

y.output: gram.y
	$(YACC) -v -y gram.y

clean:
	-rm -f pcc3 *.o y.tab.h y.output y.tab.c lex.yy.c

