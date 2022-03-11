%option noyywrap yylineno nodefault

%{
    #include "errormsg.h"
	#include "util.h"
	#include "tiger.tab.h"

	struct string curr_str = {0, 0, NULL };
%}

%x COMMENT
%x INSTRING

%%
[ \t\r]	{
	adjust(yytext); 
}

  /* newline */
\n {
	adjust(yytext); 
	EM_newline(); 
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
	adjust(yytext);
	return yytext[0];
}

 /* multi charactor punctuation symbols */
"<>"   {
	adjust(yytext);
	return NEQ;
}
"<="   {
	adjust(yytext);
	return LE;
}
">="   {
	adjust(yytext);
	return GE;
}
":="   {
	adjust(yytext);
	return ASSIGN;
}

 /* reserved words */
"array"    {
	adjust(yytext);
	return ARRAY;
}
"if"       {
	adjust(yytext);
	return IF;
}
"then"     {
	adjust(yytext);
	return THEN;
}
"else"     {
	adjust(yytext);
	return ELSE;
}
"while"    {
	adjust(yytext);
	return WHILE;
}
"for"  	 {
	adjust(yytext);
	return FOR;
}
"to"       {
	adjust(yytext);
	return TO;
}
"do"       {
	adjust(yytext);
	return DO;
}
"let"      {
	adjust(yytext);
	return LET;
}
"in"       {
	adjust(yytext);
	return IN;
}
"end"      {
	adjust(yytext);
	return END;
}
"of"       {
	adjust(yytext);
	return OF;
}
"break"    {
	adjust(yytext);
	return BREAK;
}
"nil"      {
	adjust(yytext);
	return NIL;
}
"function" {
	adjust(yytext);
	return FUNCTION;
}
"var"      {
	adjust(yytext);
	return VAR;
}
"type"     {
	adjust(yytext);
	return TYPE;
}

 /* identifier */
[a-zA-Z][a-zA-Z0-9_]* {
    adjust(yytext);
    // yytext = null-terminated string for matched token
    yylval.sval = strdup(yytext); 
    return ID;
}

 /* comments */
"/*" {
	adjust(yytext);
	BEGIN(COMMENT);
}
<COMMENT>"*/" {
	adjust(yytext);
	// INITIAL is the default begining start condition in flex
	BEGIN(INITIAL);
}
<COMMENT>([^*]|\n)+|. {
	// Equivlent to `.|\n` but
	// 1. Will match non `*` string once instead of one by one
	// 2. If `*/` is met, this rule will match only with `*`
	// 3. Thus `*/` is a longer match, which flex prefers to match
	adjust(yytext);
}
<COMMENT><<EOF>> {
	adjust(yytext);
	EM_error(EM_tokPos,"unclose comment\n");
	// Built in function
	// Terminate the scanner and return 0 to the caller of scaner
	yyterminate();
}

 /*String*/
\" {
	adjust(yytext);
	init_str(&curr_str);
	BEGIN(INSTRING);
}
<INSTRING>\"  {
	adjust(yytext); 
	yylval.sval = curr_str.data; 
	curr_str.len = 0;
	curr_str.cap = 0;
	BEGIN(INITIAL); 
	return STRING;
}
<INSTRING>\\[0-9]{3} {
	// \ddd, x is num, indicating the acsii code for a charactor
	int tmp = atoi(&yytext[1]);
	adjust(yytext);
	if (tmp > 0xff) {
		EM_error(EM_tokPos,"ascii code out of range"); 
		yyterminate();
	}
	put_str(&curr_str, (char)tmp);
}
<INSTRING>\\n {
	adjust(yytext); 
	put_str(&curr_str, '\n');
}
<INSTRING>\\t {
	adjust(yytext); 
	put_str(&curr_str, '\t');
}
<INSTRING>\\\\ {
	adjust(yytext); 
	put_str(&curr_str, '\\');
}
<INSTRING>\\\" {
	adjust(yytext); 
	put_str(&curr_str, '\"');
}
<INSTRING>\\[ \n\t\f]+\\ {
	int i = 0;
	adjust(yytext);
	while (yytext[i]) {
		if (yytext[i] == '\n') {
			EM_newline();
		}
		i++;
	}
}
<INSTRING>[^\\\n\"]* {
	adjust(yytext); 
	append_str(&curr_str, yytext);
}
<INSTRING>\n {
	adjust(yytext); 
	EM_error(EM_tokPos,"unclose string: newline appear in string"); 
	yyterminate();
}
<INSTRING><<EOF>> {
	adjust(yytext); 
	EM_error(EM_tokPos,"unclose string: EOF appear in string"); 
	yyterminate();
}


 /*Integer*/
[0-9]+ {
	adjust(yytext);
	yylval.ival = atoi(yytext);
	return INT;
}

 /* unknown input */
. {
	adjust(yytext); 
	EM_error(EM_tokPos,"illegal token");
}
%%

