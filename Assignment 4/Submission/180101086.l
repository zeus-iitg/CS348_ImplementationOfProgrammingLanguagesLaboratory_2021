%{
#include "y.tab.h"
#include "string.h"
void yyerror (const char *s);
%}


%%

"PROGRAM"					return PROGRAM;
"VAR"						return VAR;
"BEGIN"						return BEGIN_TOKEN;
"END."						return PROGRAM_END;
"END"						return END;
"INTEGER"					return INTEGER;
"REAL"						return REAL;
"FOR"						return FOR;
"READ"						return READ;
"WRITE"						return WRITE;
"TO"						return TO;
"DO"						return DO;
;							return yytext[0];
:=							return ASSIGNMENT;
[:,+*-]						return yytext[0];
"DIV"						return DIVIDE;
[()]						return yytext[0];
[a-zA-Z][_a-zA-Z0-9]*		{strcpy(yylval.id,yytext); return IDENTIFIER;}
-?[0-9]+					{yylval.int_num = atoi(yytext); return INTEGER_NUMBER;}
-?[0-9]+\.?[0-9]+			{yylval.real_num = atof(yytext); return REAL_NUMBER;}
[ \t]						;
[\n]                        {yylineno++;}
.							{ECHO; char error_message[200] = "UNEXPECTED CHARACTER "; strcat(error_message, yytext); yyerror (error_message);}

%%

int yywrap(void)
{
	return 1;
}