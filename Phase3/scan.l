%{
/*
 * lex/flex input for C scanner
 *
 */

#include <ctype.h>
#include <string.h>

#include "defs.h"
#include "types.h"
#include "symtab.h"
#include "message.h"
#include "bucket.h"
#include BACKEND_HEADER_FILE
#include "y.tab.h"

#undef yywrap

typedef enum {DEC_CONST, HEX_CONST, OCT_CONST, CHR_CONST} INT_TAG;

void count();
void comment();
double convert_double_const();
int convert_int_const(INT_TAG tag);
int escchar(char *str);
int digit(char c);

YYSTYPE yylval;
/* int yylineno = 1; */

%}

D			[0-9]
O			[0-7]
L			[a-zA-Z_]
H			[a-fA-F0-9]
E			[Ee][+-]?{D}+
FS			(f|F|l|L)
IS			(u|U|l|L)*

%%

"/*"			{ comment(); }

"auto"			{ count(); return(AUTO); }
"break"			{ count(); return(BREAK); }
"case"			{ count(); return(CASE); }
"char"			{ count(); return(CHAR); }
"const"			{ count(); return(CONST); }
"continue"		{ count(); return(CONTINUE); }
"default"		{ count(); return(DEFAULT); }
"do"			{ count(); return(DO); }
"double"		{ count(); return(DOUBLE); }
"else"			{ count(); return(ELSE); }
"enum"			{ count(); return(ENUM); }
"extern"		{ count(); return(EXTERN); }
"float"			{ count(); return(FLOAT); }
"for"			{ count(); return(FOR); }
"goto"			{ count(); return(GOTO); }
"if"			{ count(); return(IF); }
"int"			{ count(); return(INT); }
"long"			{ count(); return(LONG); }
"register"		{ count(); return(REGISTER); }
"return"		{ count(); return(RETURN); }
"short"			{ count(); return(SHORT); }
"signed"		{ count(); return(SIGNED); }
"sizeof"		{ count(); return(SIZEOF); }
"static"		{ count(); return(STATIC); }
"struct"		{ count(); return(STRUCT); }
"switch"		{ count(); return(SWITCH); }
"typedef"		{ count(); return(TYPEDEF); }
"union"			{ count(); return(UNION); }
"unsigned"		{ count(); return(UNSIGNED); }
"void"			{ count(); return(VOID); }
"volatile"		{ count(); return(VOLATILE); }
"while"			{ count(); return(WHILE); }

{L}({L}|{D})*		{	
			    count(); 
			    yylval.y_string = yytext;
			    return(IDENTIFIER); 
			}

0[xX]{H}+{IS}?		{ 
				count(); 
				yylval.y_int = convert_int_const(HEX_CONST);
				return(INT_CONSTANT); 
			}
0{O}*{IS}?		{ 
				count(); 
				yylval.y_int = convert_int_const(OCT_CONST);
				return(INT_CONSTANT); 
			}
{D}+{IS}?		{ 
				count(); 
				yylval.y_int = convert_int_const(DEC_CONST);
				return(INT_CONSTANT); 
			}
'(\\.|[^\\'])+'		{ 
				count(); 
				yylval.y_int = convert_int_const(CHR_CONST);
				return(INT_CONSTANT); 
			}
{D}+{E}{FS}?		{
				count();
				yylval.y_double = convert_double_const();
				return(DOUBLE_CONSTANT); 
			}
{D}*"."{D}+({E})?{FS}?	{ 
				count(); 
				yylval.y_double = convert_double_const();
				return(DOUBLE_CONSTANT); 
			}
{D}+"."({E})?{FS}?	{ 
				count(); 
				yylval.y_double = convert_double_const();
				return(DOUBLE_CONSTANT); 
			}
\"(\\.|[^\\"])*\"	{ 
				count(); 
				yytext [strlen (yytext) - 1] = '\0';
				yylval.y_string = st_save_string (yytext + 1);
				return(STRING_LITERAL); 
			}
"..."				{ count(); return(ELIPSIS); }
">>="				{ count(); return(RIGHT_ASSIGN); }
"<<="				{ count(); return(LEFT_ASSIGN); }
"+="				{ count(); return(ADD_ASSIGN); }
"-="				{ count(); return(SUB_ASSIGN); }
"*="				{ count(); return(MUL_ASSIGN); }
"/="				{ count(); return(DIV_ASSIGN); }
"%="				{ count(); return(MOD_ASSIGN); }
"&="				{ count(); return(AND_ASSIGN); }
"^="				{ count(); return(XOR_ASSIGN); }
"|="				{ count(); return(OR_ASSIGN); }
">>"				{ count(); return(RIGHT_OP); }
"<<"				{ count(); return(LEFT_OP); }
"++"				{ count(); return(INC_OP); }
"--"				{ count(); return(DEC_OP); }
"->"				{ count(); return(PTR_OP); }
"&&"				{ count(); return(AND_OP); }
"||"				{ count(); return(OR_OP); }
"<="				{ count(); return(LE_OP); }
">="				{ count(); return(GE_OP); }
"=="				{ count(); return(EQ_OP); }
"!="				{ count(); return(NE_OP); }
[;{},:=()[\].&!~+*/%<>^|?#-]	{ count(); return(yytext[0]); }

[ \t\v\n\f]			{ count(); }
.				{ count(); return(BAD); }

%%

int yywrap()
{
    return 1;
}

int column = 0;

void comment()
{
    char c, c1;

    column += 2;   /* for '/*' */

loop:
    while (1)
    {
        c = input();

	if (c == '*' || c == 0)
	    break;

	if (c == '\n')
	{
	    column = 0;
	    b_lineno_comment(++yylineno);
	}
	else if (c == '\t')
	    column += 8 - (column % 8);
	else
	    column++;
    }

    if (c != 0)
    {
	c1 = input();

	if (c1 != '/' && c1 != 0)
	{
	    unput(c1);
	    goto loop;
	}
    }

    if (c == 0 || c1 == 0) {
	error("open comment at EOF");
    }
    else {
	column += 2;   /* for '*' and '/' */
    }
}

void count()
{
	int i;

	for (i = 0; yytext[i] != '\0'; i++)
		{
		if (yytext[i] == '\n')
			{
			b_lineno_comment(++yylineno);
			column = 0;
			}
		else if (yytext[i] == '\t')
			column += 8 - (column % 8);
		else
			column++;
		}
}

int convert_int_const(INT_TAG tag)
{
    int ret = 0;
    char ch;

    switch (tag)
    {
      case DEC_CONST:
        sscanf (yytext, "%d", &ret);
	break;
      case HEX_CONST:
        sscanf (yytext, "%x", &ret);
	break;
      case OCT_CONST:
        sscanf (yytext, "%o", &ret);
	break;
      case CHR_CONST:
        sscanf (yytext, "'%c", &ch);
	if (ch == '\\')
	    return escchar (yytext);
	else
	    return ch;
    }

    return ret;
}

double convert_double_const()
{
    double ret;

    sscanf (yytext, "%lf", &ret);
    return ret;
}

/**************************************************************/
/* reads an ESC character from the input and then converts    */
/* it to proper character and returns it                      */
 
int escchar(char *str)
{
    char c,answer;
    int intanswer;
 
    sscanf (str, "'\\%c", &c);

    switch (c)
    {
      case '\'':
      case '"':
      case '\\':
      case '?':
        answer = c; break;
      case 'a':
        answer = '\a'; break;
      case 'b':
        answer = '\b'; break;
      case 'f' :
        answer = '\f'; break;
      case 'n':
        answer = '\n'; break;
      case 'r':
        answer = '\r'; break;
      case 't':
        answer = '\t'; break;
      case 'v':
        answer = '\v'; break;
      case 'x':
        intanswer = 0;
	for (str +=3,sscanf(str,"%c",&c);isxdigit(c);sscanf(++str,"%c", &c))
            intanswer = intanswer * 16 + digit(c);

        answer = intanswer;
        break;
      default:
        intanswer = 0;
        if (c >= '0' && c < '8')
        {
            intanswer = digit(c);
	    str += 3;
            sscanf (str, "%c", &c);

            if (c >= '0' && c < '8')
            {
                intanswer = intanswer * 8 + digit(c);
                sscanf (++str, "%c", &c);

                if (c >= '0' && c < '8')
                    intanswer = intanswer * 8 + digit(c);
            }
        }

        answer = (char)intanswer;
        break;
    }

    return answer;
}

/**************************************************************/
/* return the value of a hex digit                            */
 
int digit(char c)
{
    if (isdigit(c))
        return c-'0';

    return (toupper(c)-'A'+10);
}
