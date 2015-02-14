/* 
 * The following %-parameters
 * are the minimum sizes needed
 * by real Lex
 */
%e  1019
%p  2807
%n  371
%k  284
%a  1213
%o  1117

O   [0-7]
D   [0-9]
NZ  [1-9]
L   [a-zA-Z_]
A   [a-zA-Z_0-9]
H   [a-fA-F0-9]
HP  (0[xX])
E   ([Ee][+-]?{D}+)
P   ([Pp][+-]?{D}+)
FS  (f|F|l|L)
IS  (((u|U)(l|L|ll|LL)?)|((l|L|ll|LL)(u|U)?))
CP  (u|U|L)
SP  (u8|u|U|L)
ES  (\\(['"\?\\abfnrtv]|[0-7]{1,3}|x[a-fA-F0-9]+))
WS  [ \t\v\n\f]

%{
#include <stdio.h>
#include "y.tab.h"

extern void yyerror(const char *);  /* prints grammar violation message */

extern int sym_type(const char *);  /* returns type from symbol table */

#define sym_type(identifier) IDENTIFIER /* with no symbol table, fake it */

static void comment(void);
static int check_type(void);
%}

%%
"/*"				{ comment(); }
"//".*			{ /* consume //-comment */ }

"break"			{ return(BREAK); }
"case"			{ return(CASE); }
"char"			{ return(CHAR); }
"default"		{ return(DEFAULT); }
"do"				{ return(DO); }
"else"			{ return(ELSE); }
"goto"			{ return(GOTO); }
"if"				{ return(IF); }
"int"				{ return(INT); }
"return"		{ return(RETURN); }
"sizeof"		{ return(SIZEOF); }
"switch"		{ return(SWITCH); }
"void"			{ return(VOID); }
"while"			{ return(WHILE); }

{L}{A}*			{ return check_type(); }

{HP}{H}+{IS}?									{ return I_CONSTANT; }
{NZ}{D}*{IS}?									{ return I_CONSTANT; }
"0"{O}*{IS}?									{ return I_CONSTANT; }
{CP}?"'"([^'\\\n]|{ES})+"'"		{ return I_CONSTANT; }

({SP}?\"([^"\\\n]|{ES})*\"{WS}*)+	{ return STRING_LITERAL; }

"&&"					{ return AND_OP; }
"||"					{ return OR_OP; }
"<="					{ return LE_OP; }
">="					{ return GE_OP; }
"=="					{ return EQ_OP; }
"!="					{ return NE_OP; }
";"						{ return ';'; }
("{"|"<%")		{ return '{'; }
("}"|"%>")		{ return '}'; }
","					{ return ','; }
"="					{ return '='; }
"("					{ return '('; }
")"					{ return ')'; }
"!"					{ return '!'; }
"-"					{ return '-'; }
"+"					{ return '+'; }
"*"					{ return '*'; }
"/"					{ return '/'; }
"%"					{ return '%'; }
"<"					{ return '<'; }
">"					{ return '>'; }

{WS}					{ /* whitespace separates tokens */ }
.							{ printf("SERIOUSLY DAWG?!?"); /* discard bad characters */ }

%%

int yywrap(void)        /* called at end of input */
{
    return 1;           /* terminate now */
}

static void comment(void)
{
    int c;

    while ((c = input()) != 0)
        if (c == '*')
        {
            while ((c = input()) == '*')
                ;

            if (c == '/')
                return;

            if (c == 0)
                break;
        }
    yyerror("unterminated comment");
}

static int check_type(void)
{
		return IDENTIFIER;
}
