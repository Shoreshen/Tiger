%option noyywrap yylineno nodefault

%{
    #include "errormsg.h"
	#include "util.h"
	#include "tiger.tab.h"

	struct string curr_str = {0, 0, NULL };
	
	int yycolumn = 1;
	#define YY_USER_ACTION \
		yylloc.first_line = yylloc.last_line = yylineno; \
		yylloc.first_column = yylloc.last_column = yycolumn; \
		yycolumn += yyleng; \
		yylloc.filename = filename;
%}

%x COMMENT
%x INSTRING

%%
[ \t\r]

  /* newline */
\n {
	yycolumn = 1;
}

 /* single charactor punctuation symbols */
","	   |
":"    |
";"    |
"("    |
")"    |
"["    |
"]"    |
"{"    |
"}"    |
"."    |
"+"    |
"-"    |
"*"    |
"/"    |
"="    |
"<"    |
">"    |
"&"    |
"|"    {
	return yytext[0];
}

 /* multi charactor punctuation symbols */
"<>"   {
	return NEQ;
}
"<="   {
	return LE;
}
">="   {
	return GE;
}
":="   {
	return ASSIGN;
}

 /* reserved words */
"array"    {
	return ARRAY;
}
"if"       {
	return IF;
}
"then"     {
	return THEN;
}
"else"     {
	return ELSE;
}
"while"    {
	return WHILE;
}
"for"  	 {
	return FOR;
}
"to"       {
	return TO;
}
"do"       {
	return DO;
}
"let"      {
	return LET;
}
"in"       {
	return IN;
}
"end"      {
	return END;
}
"of"       {
	return OF;
}
"break"    {
	return BREAK;
}
"nil"      {
	return NIL;
}
"function" {
	return FUNCTION;
}
"var"      {
	return VAR;
}
"type"     {
	return TYPE;
}

 /* identifier */
[a-zA-Z][a-zA-Z0-9_]* {
    // yytext = null-terminated string for matched token
    yylval.sval = strdup(yytext); 
    return ID;
}

 /* comments */
"/*" {
	BEGIN(COMMENT);
}
<COMMENT>[^*\n]* {
	// Any string of characters that is not a '\n' or '*'
}
<COMMENT>"*"+[^*/\n]* {
	// Any string of start with multiples of '*' and
	// end with string that not contians '*', '/' or '\n'
}
<COMMENT>\n {
	yycolumn = 1;
}
<COMMENT>"*"+"/" {
	// INITIAL is the default begining start condition in flex
	BEGIN(INITIAL);
}
<COMMENT><<EOF>> {
	EM_error(EM_tokPos,"unclose comment\n");
	// Built in function
	// Terminate the scanner and return 0 to the caller of scaner
	yyterminate();
}

 /*String*/
\" {
	init_str(&curr_str);
	BEGIN(INSTRING);
}
<INSTRING>\"  {
	yylval.sval = curr_str.data; 
	curr_str.len = 0;
	curr_str.cap = 0;
	BEGIN(INITIAL); 
	return STRING;
}
<INSTRING>\\[0-9]{3} {
	// \ddd, x is num, indicating the acsii code for a charactor
	int tmp = atoi(&yytext[1]);
	if (tmp > 0xff) {
		EM_error(EM_tokPos,"ascii code out of range"); 
		yyterminate();
	}
	put_str(&curr_str, (char)tmp);
}
<INSTRING>\\n {
	put_str(&curr_str, '\n');
}
<INSTRING>\\t {
	put_str(&curr_str, '\t');
}
<INSTRING>\\\\ {
	put_str(&curr_str, '\\');
}
<INSTRING>\\\" {
	put_str(&curr_str, '\"');
}
<INSTRING>\\[ \n\t\f]+\\ {
	int i = 0;
	while (yytext[i]) {
		if (yytext[i] == '\n') {
			EM_newline();
		}
		i++;
	}
}
<INSTRING>[^\\\n\"]* {
	append_str(&curr_str, yytext);
}
<INSTRING>\n {
	EM_error(EM_tokPos,"unclose string: newline appear in string"); 
	yyterminate();
}
<INSTRING><<EOF>> {
	EM_error(EM_tokPos,"unclose string: EOF appear in string"); 
	yyterminate();
}


 /*Integer*/
[0-9]+ {
	yylval.ival = atoi(yytext);
	return INT;
}

 /* unknown input */
. {
	EM_error(EM_tokPos,"illegal token");
}
%%

