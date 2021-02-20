INSTRUCTIONS FOR RUNNING THE CODE
---------------------------------

1. The environment required for running the code is
					Linux g++
2. Open the terminal and navigate to the directory where
   the contents of this zip file have been extracted.
3. For compilation, type
				g++ two_pass_assembler.cpp
4. To run the file, type
				./a.out <name_of_input_file>
   Providing <name_of_input_file> is optional.
   If no command line argument is provided, the default filename of "input.txt" is used.
   Example:
			1) ./a.out input_file.txt
			2) ./a.out
			In the first scenario, the assembler will assume input to be stored in "input_file.txt"
			In the second scenario, the assembler will assume input to be stored in "input.txt"

INSIGHTS TO CODE
----------------

1. The assembler uses File I/O Management through C++. If any error in opening or creating files
   is encountered, the assembler displays an error message on the terminal and then halts execution.

2. The name of intermediate file is set to "intermediate.txt" and that of output file is set to
   "output.txt". The name of input file provided should not match with these two filenames. If it does,
   the assembler displays an error message on the terminal and then halts execution.

3. If there is any error in the input file (duplicate symbol, undefined symbol, invalid operation code),
   the assembler displays an error message on the terminal, deletes any files that it has created
   and then halts execution.

4. This assembler handles multiple special cases (comments, empty lines, whitespaces in character
   constant).

5. The assembler issues warnings to the user if there are odd number of characters in the hexadecimal
   constant. But it does not halt execution in this case and automatically inserts a '0' at the
   beginning of the hexadecimal constant and continues execution.

6. Since the assembler uses stringstream for taking input, no particular format of instructions is
   required. The only condition is that every instruction must be on a separate line.
   
7. For any queries, kindly contact me at vaibhav18a@iitg.ac.in