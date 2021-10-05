%option noyywrap yylineno nodefault

%{
    #include "tokens.h"
    #include "errormsg.h"
	#include "util.h"

	struct string curr_str = {0, 0, NULL };
%}

%x COMMENT
%x INSTRING

%%
 /* punctuation symbols */
","	   {
	adjust();
	return COMMA;
}
":"    {
	adjust();
	return COLON;
}
";"    {
	adjust();
	return SEMICOLON;
}
"("    {
	adjust();
	return LPAREN;
}
")"    {
	adjust();
	return RPAREN;
}
"["    {
	adjust();
	return LBRACK;
}
"]"    {
	adjust();
	return RBRACK;
}
"{"    {
	adjust();
	return LBRACE;
}
"}"    {
	adjust();
	return RBRACE;
}
"."    {
	adjust();
	return DOT;
}
"+"    {
	adjust();
	return PLUS;
}
"-"    {
	adjust();
	return MINUS;
}
"*"    {
	adjust();
	return TIMES;
}
"/"    {
	adjust();
	return DIVIDE;
}
"="    {
	adjust();
	return EQ;
}
"<>"   {
	adjust();
	return NEQ;
}
"<"    {
	adjust();
	return LT;
}
"<="   {
	adjust();
	return LE;
}
">"    {
	adjust();
	return GT;
}
">="   {
	adjust();
	return GE;
}
"&"    {
	adjust();
	return AND;
}
"|"    {
	adjust();
	return OR;
}
":="   {
	adjust();
	return ASSIGN;
}

 /* reserved words */
"array"    {
	adjust();
	return ARRAY;
}
"if"       {
	adjust();
	return IF;
}
"then"     {
	adjust();
	return THEN;
}
"else"     {
	adjust();
	return ELSE;
}
"while"    {
	adjust();
	return WHILE;
}
"for"  	 {
	adjust();
	return FOR;
}
"to"       {
	adjust();
	return TO;
}
"do"       {
	adjust();
	return DO;
}
"let"      {
	adjust();
	return LET;
}
"in"       {
	adjust();
	return IN;
}
"end"      {
	adjust();
	return END;
}
"of"       {
	adjust();
	return OF;
}
"break"    {
	adjust();
	return BREAK;
}
"nil"      {
	adjust();
	return NIL;
}
"function" {
	adjust();
	return FUNCTION;
}
"var"      {
	adjust();
	return VAR;
}
"type"     {
	adjust();
	return TYPE;
}

 /* identifier */
[a-zA-Z][a-zA-Z0-9_]* {
    adjust();
    // yytext = null-terminated string for matched token
    yylval.sval = strdup(yytext); 
    return ID;
}

 /* comments */
"/*" {
	adjust();
	BEGIN(COMMENT);
}
<COMMENT>"*/" {
	adjust();
	// INITIAL is the default begining start condition in flex
	BEGIN(INITIAL);
}
<COMMENT>([^*]|\n)+|. {
	// Equivlent to `.|\n` but
	// 1. Will match non `*` string once instead of one by one
	// 2. If `*/` is met, this rule will match only with `*`
	// 3. Thus `*/` is a longer match, which flex prefers to match
	adjust();
}
<COMMENT><<EOF>> {
	adjust();
	EM_error(EM_tokPos,"unclose comment\n");
	// Built in function
	// Terminate the scanner and return 0 to the caller of scaner
	yyterminate();
}

 /*String*/
\" {
	adjust();
	init_str(&curr_str);
	BEGIN(INSTRING);
}
<INSTRING>\"  {
	adjust(); 
	yylval.sval = curr_str.data; 
	free(curr_str.data);
	curr_str.len = 0;
	curr_str.cap = 0;
	BEGIN(INITIAL); 
	return STRING;
}
<INSTRING>\\[0-9]{3} {
	// \ddd, x is num, indicating the acsii code for a charactor
	int tmp = atoi(&yytext[1]);
	adjust();
	if (tmp > 0xff) {
		EM_error(EM_tokPos,"ascii code out of range"); 
		yyterminate();
	}
	put_str(&curr_str, (char)tmp);
}
<INSTRING>\\n {
	adjust(); 
	put_str(&curr_str, '\n');
}
<INSTRING>\\t {
	adjust(); 
	put_str(&curr_str, '\t');
}
<INSTRING>\\\\ {
	adjust(); 
	put_str(&curr_str, '\\');
}
<INSTRING>\\\" {
	adjust(); 
	put_str(&curr_str, '\"');
}
<INSTRING>\\[ \n\t\f]+\\ {
	int i = 0;
	adjust();
	while (yytext[i]) {
		if (yytext[i] == '\n') {
			EM_newline();
		}
		i++;
	}
}
<INSTRING>[^\\\n\"]* {
	adjust(); 
	append_str(&curr_str, yytext);
}
<INSTRING>\n {
	adjust(); 
	EM_error(EM_tokPos,"unclose string: newline appear in string"); 
	yyterminate();
}
<INSTRING><<EOF>> {
	adjust(); 
	EM_error(EM_tokPos,"unclose string: EOF appear in string"); 
	yyterminate();
}


 /*Integer*/
[0-9]+ {
	adjust();
	yylval.ival = atoi(yytext);
	return INT;
}

 /* unknown input */
. {
	adjust(); 
	EM_error(EM_tokPos,"illegal token");
}
%%