%%

"PROGRAM"                   return 1;
"VAR"                       return 2;
"BEGIN"                     return 3;
"END."                      return 5;
"END"                       return 4;
"INTEGER"                   return 6;
"FOR"                       return 7;
"READ"                      return 8;
"WRITE"                     return 9;
"TO"                        return 10;
"DO"                        return 11;
;                           return 12;
:=                          return 15;
:                           return 13;
,                           return 14;
\+                          return 16;
-                           return 17;
\*                          return 18;
"DIV"                       return 19;
\(                          return 20;
\)                          return 21;
[a-zA-Z][_a-zA-Z0-9]*       return 22;
-?[0-9]+                    return 23;
[ \t]                       ;
[\n]                        yylineno++;
.                           printf("UNKNOWN TOKEN\n");

%%

int yywrap(void)
{
    return 1;
}