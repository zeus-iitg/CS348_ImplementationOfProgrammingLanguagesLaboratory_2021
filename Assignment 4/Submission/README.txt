Assignment 4
Implementation of Programming Languages Lab
Parser based on Yacc
Name:    Vaibhav Kumar Singh
Roll No: 180101086
INSTRUCTIONS FOR RUNNING THE CODE
---------------------------------
1. The environment required for running the code is
					Linux gcc
2. Open the terminal and navigate to the directory where
   the contents of this zip file have been extracted.
3. Then run the following commands on the terminal
		yacc 180101086.y -d -Wno-yacc
		lex 180101086.l
		gcc lex.yy.c y.tab.c -o 180101086
		./180101086 < input.txt
4. All the syntax and semantic errors will be displayed on the terminal.
