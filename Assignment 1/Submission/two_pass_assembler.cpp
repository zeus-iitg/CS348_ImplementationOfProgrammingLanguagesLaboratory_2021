/*
Assignment 1
Implementation of Programming Languages Lab
Two Pass Assembler
Name:    Vaibhav Kumar Singh
Roll No: 180101086

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
*/

#include <bits/stdc++.h>

using namespace std;

//file streams for file I/O management
ifstream fin;
ofstream fout;

//stores the name of input file
string input_filename = "input.txt";

//stores lines read from input file
string line;

//stores all words present in
//the line read from input file
vector<string> word_list;

//stores individual words
string word;

//stores the starting address
//of the program in hexadecimal
string starting_address;

//stores the name of program
string program_name;

//Operation Code Table
map<string, string> optab;

//Symbol Table
map<string, string> symtab;

//stores the length of program
string program_length;

//Location Counter
long long locctr;

//stores the line number of
//the line read from input file
int line_num;

//used to indicate error
bool error_flag;

//stores the error message
string error_message;

//used to handle cases where
//there are whitespaces in the
//character constant
//for example
//C'X Y Z'
void handle_special_case(){
    int sz = word_list.size();
    bool specialCASE = false;
    int idx;
    for(int i = 0; i < sz; i++){
        if(word_list[i].length() > 2){
            if(word_list[i].substr(0, 2) == "C'"){
                if(word_list[i].back() != '\''){
                    specialCASE = true;
                    idx = i;
                }
            }
        }
    }
    if(specialCASE){
        vector<string> new_word_list;

        for(int i = 0; i < idx; i++){
            new_word_list.push_back(word_list[i]);
        }

        string new_word = "";
        int last;
        for(int i = idx; i < sz; i++){
            last = i;
            new_word.append(word_list[i]);
            if(word_list[i].back() == '\'')
                break;
            else{
                new_word.push_back(' ');
            }
        }
        new_word_list.push_back(new_word);

        for(int i = last + 1; i < sz; i++){
            new_word_list.push_back(word_list[i]);
        }

        word_list = new_word_list;
    }
    return;
}

//generates list of words present
//in the line read from input file
void generate_wordlist(){
    word_list.clear();
    stringstream ss(line);
    while(ss>>word){
        word_list.push_back(word);
    }
    handle_special_case();
    return;
}

//populates Operation Code Table
void fill_optab(){
    while(!fin.eof()){
        getline(fin, line);
        generate_wordlist();
        optab[word_list[0]] = word_list[1];
    }
    return;
}

//checks if the given line is empty
//line containing any character other than
//whitespace is considered to be non empty
bool checkEMPTY(){
    for(char u:line){
        if(u != ' ')
            return false;
    }
    return true;
}

//reads a line from input file
void input_line(){
    while(1){
        getline(fin, line);
        line_num++;
        if(!checkEMPTY())
            break;
    }
    return;
}

//writes given line to output file
void output_line(string &line){
    fout<<line<<endl;
    return;
}

//checks if given string is "START"
bool isSTART(string &str){
    if(str == "START")
        return true;
    return false;
}

//checks if given string is "END"
bool isEND(){
    for(string &u: word_list)
        if(u == "END")
            return true;
    return false;
}

//checks if given string is a comment
bool isCOMMENT(){
    if(word_list[0][0] == '.')
        return true;
    return false;
}

//searches Operation Code Table for mnemonic
//returns Operation Code if mnemonic is found
//else returns an empty string
string search_optab(string &mnemonic){
    if(optab.find(mnemonic) != optab.end())
        return optab[mnemonic];
    else return "";
}

//checks if given string is a label
//a string is considered a label if
//it does not coincide with any
//of the reserved words
bool isLABEL(string &label){
    if(search_optab(label) != "")
        return false;
    else{
        if(label == "WORD" || label == "RESW" ||
            label == "RESB" || label == "BYTE" ||
            label == "START" || label == "END")
            return false;
        else return true;
    }
}

//checks if this label has already
//been used previously
bool isDUPLICATE(string &label){
    if(symtab.find(label) != symtab.end())
        return true;
    return false;
}

//array storing hexadecimal codes
//for decimal numbers 0 - 15
char hex_code[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

//converts the given number
//into hexadecimal string
string hex_converter(long long num){
    if(num == 0)
        return "0";
    string res = "";
    while(num){
        res.push_back(hex_code[num%16]);
        num = num/16;
    }
    reverse(res.begin(), res.end());
    return res;
}

//makes the given string to the specified length
//if string length equals specified length
//it returns the same string
//if string length is greater than specified length
//it returns the substring containing the first 'len' characters of the given string
//if string length is less than specified length
//it appends appending_character to the specified position until string length becomes equal to specified length
//appending position 0 denotes append to beginning of string
//appending position 1 denotes append to end of string
string make_len(string &str, int len, int appending_position, char appending_character){
    string res = "";
    if((int)str.length() == len)
        res = str;
    else if((int)str.length() > len)
        res = str.substr(0, len);
    else{
        res = str;
        if(appending_position == 0){
            reverse(res.begin(), res.end());
            while((int)res.length() < len){
                res.push_back(appending_character);
            }
            reverse(res.begin(), res.end());
        }
        else{
            while((int)res.length() < len){
                res.push_back(appending_character);
            }
        }
    }
    return res;
}

//checks if Indexed Addressing Mode is specified in operand
bool hasIndexedAddressingMode(string &operand){
    if(operand.length() < 3)
        return false;
    else{
        if(operand.substr(operand.length() - 2, 2) == ",X")
            return true;
        return false;
    }
}

//retrieves symbol name from operand
string retrieveSymbolName(string &operand){
    string res = operand;
    for(int i = 0; i < 2; i++)
        res.pop_back();
    return res;
}

//handles errors in input file
void handle_error(int pass, string& error_message, int line_num, string error_word = ""){
    fin.close();
    fout.close();
    if(error_word.length())
        cout<<error_message<<" "<<error_word<<" ON LINE NUMBER "<<line_num<<"\n";
    else
        cout<<error_message<<" ON LINE NUMBER "<<line_num<<"\n";
    cout<<"EXITING PROGRAM\n";
    if(pass == 1){
        remove("intermediate.txt");
    }
    else{
        remove("intermediate.txt");
        remove("output.txt");
    }
    exit(0);
    return;
}

//handles errors in file handling
void handle_file_error(int type, string filename){
    if(type == 1)
        cout<<"FILE "<<filename<<" NOT FOUND\n";
    else
        cout<<"CREATION OF "<<filename<<" FAILED\n";
    cout<<"EXITING PROGRAM\n";
    exit(0);
}

//handles cases where filename of input coincides
//with filename of intermediate file or output file
void handle_collision(string& filename){
    if(filename == "intermediate.txt" || filename == "output.txt")
    {
        cout<<"Invalid filename\n";
        cout<<"Kindly specify filename other than \"intermediate.txt\" and \"output.txt\"\n";
        cout<<"EXITING PROGRAM\n";
        exit(0);
    }
}

//handles comments at file beginning
void handle_comments_at_beginning_of_file(int pass){
    while(!fin.eof() && isCOMMENT()){
        if(pass == 1){
            line = "     " + line;
            output_line(line);
        }
        input_line();
        generate_wordlist();
    }
}

int main(int argc, char* argv[])
{
    //checks if filename of input file has been specified by user
    if(argc >= 2){
        input_filename = argv[1];
        handle_collision(input_filename);
        cout<<"Using specified input filename: "<<input_filename<<"\n";
    }
    else{
        cout<<"Filename of input file not specified\n";
        cout<<"Using default input filename: "<<input_filename<<"\n";
    }

    //opens opcodes.txt and populates Operation Code Table
    fin.open("opcodes.txt");
    if(!fin){
        handle_file_error(1, "opcodes.txt");
    }
    fill_optab();
    fin.close();

    //opens input and intermediate files
    fin.open(input_filename);
    if(!fin){
        handle_file_error(1, input_filename);
    }
    line_num = 0;
    fout.open("intermediate.txt");
    if(!fout){
        handle_file_error(2, "intermediate.txt");
    }

    //PASS 1 begins

    //read first input line
    input_line();
    generate_wordlist();
    handle_comments_at_beginning_of_file(1);

    //if first word is label
    if(isLABEL(word_list[0])){
        //if OPCODE = 'START'
        if(isSTART(word_list[1])){
            //save LABEL in program name
            program_name = word_list[0];

            //save #[OPERAND] as starting address
            starting_address = word_list[2];

            //initialize LOCCTR to starting address
            //here we store LOCCTR in decimal, i.e. base 10
            locctr = stoll(starting_address, nullptr, 16);

            //write line to intermediate file
            line = hex_converter(locctr) + " " + line;
            output_line(line);

            //read next input line
            input_line();
            generate_wordlist();
        }
        //if OPCODE != 'START'
        else{
            //save empty string in program name
            program_name = "";

            //save 0 as starting address
            starting_address = "0";

            //initialize LOCCTR to 0
            locctr = 0;
        }
    }
    //if first word is not label
    else{
        //if OPCODE = 'START'
        if(isSTART(word_list[0])){
            //save empty string in program name
            program_name = "";

            //save #[OPERAND] as starting address
            starting_address = word_list[1];

            //initialize LOCCTR to starting address
            //here we store LOCCTR in decimal, i.e. base 10
            locctr = stoll(starting_address, nullptr, 16);

            //write line to intermediate file
            line = hex_converter(locctr) + " " + line;
            output_line(line);

            //read next input line
            input_line();
            generate_wordlist();
        }
        //if OPCODE != 'START'
        else{
            //save empty string in program name
            program_name = "";

            //save 0 as starting address
            starting_address = "0";

            //initialize LOCCTR to 0
            locctr = 0;
        }
    }


    //while OPCODE != 'END'
    while(!isEND()){
        //save value of LOCCTR for this line
        long long instruction_start_address = locctr;

        //if this is a comment line
        if(isCOMMENT())
        {
            //write line to intermediate file
            line = "     " + line;
            output_line(line);

            //read next input line
            input_line();
            generate_wordlist();
            continue;
        }

        //if this is not a comment line
        if(!isCOMMENT()){
            int idx = 0;

            //if there is a symbol in the LABEL field
            if(isLABEL(word_list[0])){
                idx++;
                string label = word_list[0];

                //search SYMTAB for LABEL
                //if LABEL is found
                if(isDUPLICATE(label)){
                    //set error flag (duplicate symbol)
                    error_flag = true;
                    error_message = "DUPLICATE SYMBOL";
                    handle_error(1, error_message, line_num, label);
                }
                //if LABEL is not found
                else{
                    //insert (LABEL,LOCCTR) into SYMTAB
                    symtab[label] = hex_converter(locctr);
                }
            }

            if(idx < (int)word_list.size()){
                string mnemonic = word_list[idx];

                //search OPTAB for OPCODE
                //if OPCODE is found
                if(search_optab(mnemonic) != ""){
                    //add 3 (instruction length) to LOCCTR
                    locctr = locctr + 3;
                }
                //if OPCODE = 'WORD'
                else if(mnemonic == "WORD"){
                    //add 3 to LOCCTR
                    locctr = locctr + 3;
                }
                //if OPCODE = 'RESW'
                else if(mnemonic == "RESW"){
                    string operand = word_list[idx + 1];

                    //add 3 * #[OPERAND] to LOCCTR
                    locctr = locctr + 3 * stoll(operand);
                }
                //if OPCODE = 'RESB'
                else if(mnemonic == "RESB"){
                    string operand = word_list[idx + 1];

                    //add #[OPERAND] to LOCCTR
                    locctr = locctr + stoll(operand);
                }
                //if OPCODE = 'BYTE'
                else if(mnemonic == "BYTE"){
                    string operand = word_list[idx + 1];

                    //if it is character constant
                    if(operand[0] == 'C'){
                        //add length of constant to LOCCTR
                        //since each character is 1 byte,
                        //total size of constant in bytes is
                        //equal to number of characters
                        locctr = locctr + operand.length() - 3;
                    }

                    //if it is hexadecimal constant
                    else if(operand[0] == 'X'){
                        //add length of constant to LOCCTR
                        //since each hexadecimal digit is 1/2 byte,
                        //total size of constant in bytes is
                        //equal to ceiling of (number of digits)/2
                        int constant_length = operand.length() - 3;
                        if((operand.length() - 3) % 2 == 1){
                            cout<<"WARNING: Hexadecimal constant of odd length on line number "<<line_num<<"\n";
                            cout<<"Appending '0' at beginning\n";
                            int pos;
                            for(int i = 0; i < (int)line.length() - 1; i++){
                                if(line[i] == 'X' && line[i+1] == '\''){
                                    pos = i + 2;
                                    break;
                                }
                            }
                            line.insert(pos, "0");
                            constant_length++;
                        }
                        locctr = locctr + constant_length / 2;
                    }
                }
                else{
                    //set error flag (invalid operation code)
                    error_flag = true;
                    error_message = "INVALID OPERATION CODE";
                    handle_error(1, error_message, line_num);
                }
            }
            //to take care of cases where there is only a single
            //non reserved word in the line. It will be treated as
            //a label. But there would not be any mnemonic after this.
            //Ex. a single line containing RSUN (RSUB was misspelt)
            else{
                //set error flag (invalid operation code)
                error_flag = true;
                error_message = "INVALID OPERATION CODE";
                handle_error(1, error_message, line_num);
            }
        }

        line = hex_converter(instruction_start_address) + " " + line;

        //write line to intermediate file
        output_line(line);

        //read next input line
        input_line();
        generate_wordlist();
    }

    line = hex_converter(locctr) + " " + line;

    //write last line to intermediate file
    output_line(line);

    //save (LOCCTR - starting address) as program length
    program_length = hex_converter(locctr - stoll(starting_address, nullptr, 16));

    //PASS 1 ends

    //close the opened files
    fin.close();
    fout.close();

    //opens intermediate and output files
    fin.open("intermediate.txt");
    if(!fin){
        handle_file_error(1, "intermediate.txt");
    }
    line_num = 0;
    fout.open("output.txt");
    if(!fout){
        handle_file_error(2, "output.txt");
    }

    //PASS 2 begins

    //read first input line (from intermediate file)
    input_line();
    generate_wordlist();
    handle_comments_at_beginning_of_file(2);

    //if second word is label
    if(isLABEL(word_list[1])){
        //if OPCODE = 'START'
        if(isSTART(word_list[2])){
            line = "H" + make_len(program_name, 6, 1, ' ') +
                    make_len(starting_address, 6, 0, '0') +
                    make_len(program_length, 6, 0, '0');

            //write Header record to object program
            output_line(line);

            //read next input line
            input_line();
            generate_wordlist();
        }
        //if OPCODE != 'START'
        else{
            line = "H" + make_len(program_name, 6, 1, ' ') +
                    make_len(starting_address, 6, 0, '0') +
                    make_len(program_length, 6, 0, '0');

            //write Header record to object program
            output_line(line);
        }
    }
    //if second word is not label
    else{
        //if OPCODE = 'START'
        if(isSTART(word_list[1])){
            line = "H" + make_len(program_name, 6, 1, ' ') +
                    make_len(starting_address, 6, 0, '0') +
                    make_len(program_length, 6, 0, '0');

            //write Header record to object program
            output_line(line);

            //read next input line
            input_line();
            generate_wordlist();
        }
        //if OPCODE != 'START'
        else{
            line = "H" + make_len(program_name, 6, 1, ' ') +
                    make_len(starting_address, 6, 0, '0') +
                    make_len(program_length, 6, 0, '0');

            //write Header record to object program
            output_line(line);
        }
    }

    //initialize first Text record
    bool not_started = true;
    string record_starting_address;
    string object_code_string = "";

    //while OPCODE != 'END'
    while(!isEND()){
        //if this is not a comment line
        if(!isCOMMENT()){
            int idx = 1;
            //if second word is label
            if(isLABEL(word_list[idx])){
                idx++;
            }

            string mnemonic = word_list[idx];

            //search OPTAB for OPCODE
            //if OPCODE is found
            if(search_optab(mnemonic) != ""){
                string operand_address;

                //if there is a symbol in operand field
                if((int)word_list.size() > idx + 1){
                    string operand = word_list[idx + 1];
                    string symbolName;

                    //check if Indexed Addressing Mode has been specified in the operand
                    bool Indexed = hasIndexedAddressingMode(operand);
                    if(Indexed)
                        symbolName = retrieveSymbolName(operand);
                    else symbolName = operand;

                    //search SYMTAB for OPERAND
                    //if OPERAND is found
                    if(symtab.find(symbolName) != symtab.end()){
                        //store symbol value as operand address
                        operand_address = symtab[symbolName];
                        operand_address = make_len(operand_address, 4, 0, '0');
                        if(Indexed)
                            operand_address[0] += 8;
                    }
                    //if OPERAND is not found
                    else{
                        //store 0 as operand address
                        operand_address = "0";
                        operand_address = make_len(operand_address, 4, 0, '0');

                        //set error flag (undefined symbol)
                        error_flag = true;
                        error_message = "UNDEFINED SYMBOL";
                        handle_error(2, error_message, line_num, symbolName);
                    }
                }
                //if there is no symbol in operand field
                else{
                    //store 0 as operand address
                    operand_address = "0";
                    operand_address = make_len(operand_address, 4, 0, '0');
                }

                //assemble the object code instruction
                string object_code_instruction = optab[mnemonic] + operand_address;

                //if object code can fit into the current Text record
                if(object_code_string.length() + object_code_instruction.length() <= 60){
                    //add object to Text record
                    object_code_string += object_code_instruction;

                    //if record has not started yet, set starting
                    //address to LOCCTR of current instruction
                    if(not_started){
                        record_starting_address = word_list[0];
                        not_started = false;
                    }
                }
                //if object code will not fit into the current Text record
                else{
                    string length_of_current_record = hex_converter(object_code_string.length()/2);
                    string Text_record = "T" + make_len(record_starting_address, 6, 0, '0') +
                                        make_len(length_of_current_record, 2, 0, '0') +
                                        object_code_string;

                    //write Text record to object program
                    output_line(Text_record);

                    //initialize new Text record
                    record_starting_address = word_list[0];
                    object_code_string = object_code_instruction;
                }
            }
            //if OPCODE = 'BYTE' or 'WORD'
            else if(mnemonic == "BYTE" || mnemonic == "WORD"){
                string object_code_of_constant = "";

                //if OPCODE = 'BYTE'
                if(mnemonic == "BYTE"){
                    string constant = word_list[idx + 1];

                    //if it is a character constant
                    if(constant[0] == 'C'){
                        //object code for a character is its ASCII equivalent
                        //converted into a 2-digit hexadecimal value
                        //append values of all characters together to obtain
                        //object code for the character constant
                        constant = constant.substr(2, constant.length() - 3);
                        for(char u:constant){
                            long long num = u;
                            string hexadecimal_value = hex_converter(num);
                            object_code_of_constant += make_len(hexadecimal_value, 2, 0, '0');
                        }
                    }
                    //if it is a hexadecimal constant
                    else if(constant[0] == 'X'){
                        //object code for hexadecimal constant
                        //is the value of the constant itself
                        constant = constant.substr(2, constant.length() - 3);

                        //convert constant to object code
                        object_code_of_constant = constant;
                    }
                }
                //if OPCODE = 'WORD'
                else if(mnemonic == "WORD"){
                    long long num = stoll(word_list[idx + 1]);

                    //convert constant to object code
                    object_code_of_constant = hex_converter(num);
                    object_code_of_constant = make_len(object_code_of_constant, 6, 0, '0');
                }

                //if object code can fit into the current Text record
                if(object_code_string.length() + object_code_of_constant.length() <= 60){
                    //add object to Text record
                    object_code_string += object_code_of_constant;

                    //if record has not started yet, set starting
                    //address to LOCCTR of current instruction
                    if(not_started){
                        record_starting_address = word_list[0];
                        not_started = false;
                    }
                }
                //if object code will not fit into the current Text record
                else{
                    string length_of_current_record = hex_converter(object_code_string.length()/2);
                    string Text_record = "T" + make_len(record_starting_address, 6, 0, '0') +
                                        make_len(length_of_current_record, 2, 0, '0') +
                                        object_code_string;

                    //write Text record to object program
                    output_line(Text_record);

                    //initialize new Text record
                    record_starting_address = word_list[0];
                    object_code_string = object_code_of_constant;
                }
            }
            //if OPCODE = 'RESB' or 'RESW'
            else{
                //if there is a non-empty Text record
                if(object_code_string != ""){
                    string length_of_current_record = hex_converter(object_code_string.length()/2);
                    string Text_record = "T" + make_len(record_starting_address, 6, 0, '0') +
                                        make_len(length_of_current_record, 2, 0, '0') +
                                        object_code_string;

                    //write Text record to object program
                    output_line(Text_record);

                    //initialize new empty Text record
                    //set Text record as not started
                    object_code_string = "";
                    not_started = true;
                }
            }
        }

        //read next input line
        input_line();
        generate_wordlist();
    }

    //write last Text record to object program
    if(object_code_string != ""){
        string length_of_current_record = hex_converter(object_code_string.length()/2);
        string last_Text_record = "T" + make_len(record_starting_address, 6, 0, '0') +
                            make_len(length_of_current_record, 2, 0, '0') +
                            object_code_string;
        output_line(last_Text_record);
    }

    //write End record to object program
    string End_record = "E" + make_len(starting_address, 6, 0, '0');
    output_line(End_record);

    //PASS 2 ends

    return 0;
}
