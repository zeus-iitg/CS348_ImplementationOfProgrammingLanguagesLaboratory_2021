                                                                           
                                                                           TWO PASS ASSEMBLER
                                                                           ==================

INSTRUCTIONS FOR RUNNING THE CODE
---------------------------------
1. The environment required for running the code is
					Linux g++
2. Open the terminal and navigate to the directory where
   the contents of this zip file have been extracted.
3. For compilation, type
				g++ assembler.cpp
4. To run the file, type
				./a.out <name_of_input_file>
   Providing <name_of_input_file> is optional.
   If no command line argument is provided, the default filename of "input.txt" is used.
   Example:
			1) ./a.out input_file.txt
			2) ./a.out
			In the first scenario, the assembler will assume input to be stored in "input_file.txt"
			In the second scenario, the assembler will assume input to be stored in "input.txt"

FORMAT OF INPUT FILE
--------------------
Every line should have between 21 and 80 columns (inclusive).
First 10 columns for LABEL, next 10 columns for OPCODE and (at max) the next 60 for OPERAND
                                                                           
                                                                            LINKING LOADER
                                                                            ==============

INSTRUCTIONS FOR RUNNING THE CODE
---------------------------------
1. The environment required for running the code is
					Linux g++
2. Open the terminal and navigate to the directory where
   the contents of this zip file have been extracted.
3. For compilation, type
				g++ linking_loader.cpp
4. To run the file, type
				./a.out <name_of_assembler_intermediate_file> <name_of_assembler_output_file>
   Providing <name_of_assembler_intermediate_file> and <name_of_assembler_output_file> is optional.
   If no command line argument is provided, the default filenames ("assembler_intermediate.txt" and "assembler_output.txt") are used.
   Example:
			1) ./a.out intermediate.txt output.txt
			2) ./a.out
			In the first scenario, the assembler will assume
                            assembler intermediate filename to be "intermediate.txt"
                            assembler output filename to be "output.txt"
			In the second scenario, the assembler will assume
                            assembler intermediate filename to be "assembler_intermediate.txt"
                            assembler output filename to be "assembler_output.txt"