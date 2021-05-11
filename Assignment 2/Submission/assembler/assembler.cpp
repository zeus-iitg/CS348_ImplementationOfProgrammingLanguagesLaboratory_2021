/*
Assignment 2
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
*/

#include <bits/stdc++.h>

using namespace std;

//Location Counter
int LOCCTR;

//starting address of program
int start_address;

//length of program
int programLength;

//strings for label, opcode and operand respectively
string label;
string opcode;
string operand;

//true for extended format instruction
//false otherwise
bool extendedFlag;

//input stream
ifstream fin;

//output stream
ofstream fout;

//name of input file
string input_filename;

//string for storing a line of input
string line;

//string for storing a word in a line
string word;

//vector for storing all words in the input line
vector<string> word_list;

//Operation Code Table
map<string,string> OPTAB;

//generates list of words present
//in the line read from input file
void generate_wordlist(){
    word_list.clear();
    stringstream ss(line);
    while(ss>>word){
        word_list.push_back(word);
    }
    return;
}

//populates Operation Code Table
void fill_optab(){
    while(!fin.eof()){
        getline(fin, line);
        generate_wordlist();
        OPTAB[word_list[0]] = word_list[1];
    }
    return;
}

//error flags used for indicating error in input
//used in evaluateExpression()
bool SYNTAX_ERROR;
bool UNDEFINED_SYMBOL;

//Symbol Table
//Table mapping symbols to their addresses and error flags.
//Predefined symbols are registers.
//Secondary map for section name.
map<string, map<string,string>> SYMTAB;

//populates Symbol Table
void fill_symtab(){
    while(!fin.eof()){
        getline(fin, line);
        generate_wordlist();
        map<string, string> temp;
        temp[word_list[1]]=word_list[2];
        SYMTAB[word_list[0]] = temp;
    }
    return;
}

//Literal Table
map<string,string> LITTAB;

//set of EXTREF symbols
set<string> EXTREF_SYMBOLS;

//Modification Records
vector<string> MODIF_RECORDS;

//map storing (Program name, length) pairs
map<string,int> progNames;

//string storing Program name
string progName;

//Base value (0 by default)
int base_register_value;

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

//Remove spaces from beginning and end of string
string stripString(string s){
	//Removing spaces from end
	while(!s.empty() && s.back()==' ')
        s.pop_back();
	if(s.empty())
        return s;
	//Removing spaces from beginning
	reverse(s.begin(), s.end());
	while(!s.empty() && s.back()==' ')
        s.pop_back();
    reverse(s.begin(), s.end());
	return s;
}

//Retrieve file name without extension so output file name can be created
string getFileWithoutExtension(string s){
	size_t found = s.find(".");
	if (found != string::npos)
        return s.substr(0,found);
	else return s;
}

//Extract opcode, label and operand from a line based on the input file format given at the top
bool disectLine(string &line, string &label, string &opcode, string &operand, bool &extendedFlag){
    //if line comprises only of spaces
	if(stripString(line).empty()){
		return false;
	}
	//Checking if valid line and not comment
	if((line.size()>80 || line.size()<21) && stripString(line)[0]!='.'){
		return false;
	} else if(stripString(line)[0]=='.'){
		//Comments are still printed in intermediate.txt
		return true;
	}
	//Adding spaces at the end of line to make it 80 characters long.
	line = make_len(line, 80, 1, ' ');
	//extracting label, opcode and operand fields from input based on input file format specified at top
	//and then removing spaces at the beginning and end of these fields
	label=stripString(line.substr(0,10));
	opcode=stripString(line.substr(10,10));
	operand=stripString(line.substr(20));

	//Handle extended format
	if(opcode[0]=='+'){
		extendedFlag=true;
		reverse(opcode.begin(), opcode.end());
		opcode.pop_back();
		reverse(opcode.begin(), opcode.end());
	}
	else {
		extendedFlag=false;
	}

	return true;
}

//array storing hexadecimal codes
//for decimal numbers 0 - 15
char hex_code[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

//converts the given number
//into hexadecimal string
string dec_to_hex(int num){
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

//Convert hexadecimal string to decimal integer
int hex_to_dec(string hex){
	int res=0;
	for(int i=0;i<(int)hex.length();i++){
		if(hex[i]>='A' && hex[i]<='F')
            res = res*16 + hex[i] - 'A' + 10;
		else
            res = res*16 + hex[i] - '0';
	}
	return res;
}

//returns true if input string only contains digits
//returns false otherwise
bool isInteger(string s){
	for(auto u:s){
		if(u < '0' || u > '9')
			return false;
	}
	return true;
}

//Break a byte into its hexadecimal parts
//Used for converting C'<string>' into hexadecimal characters
string getHexBreakup(string s){
	string final;
	for(int i=0;i<(int)s.length();i++){
		//Break into two hexadecimal codes
		int x=(s[i]>>4);
		int y=s[i]-(x<<4);
		final.append(dec_to_hex(x));
		final.append(dec_to_hex(y));
	}
	return final;
}
//Used to generate line if either label, opcode or operand are changed. Used in hexadecimal string padding.
string createLine(string label, string opcode, string operand){
	while(label.length()<10)label+=' ';
	while(opcode.length()<10)opcode+=' ';
	while(operand.length()<60)operand+=' ';
	return label+opcode+operand;
}

//function for handling errors and warnings in input file
//TYPE 1 - SYNTAX ERROR
//TYPE 2 - UNDEFINED SYMBOL ERROR
//TYPE 3 - INVALID OPERATION CODE ERROR
//TYPE 4 - COMMA WARNING
//TYPE 5 - DUPLICATE SYMBOL ERROR
//TYPE 6 - ILLEGAL EXTDEF ERROR
//TYPE 7 - DISPLACEMENT OUT OF BOUNDS ERROR
//TYPE 8 - LITTAB VALUES NOT DEFINED ERROR
void handle_errors(int type, string line = ""){
    if(type == 1){
        cout<<"SYNTAX ERROR IN THE FOLLOWING LINE\n";
        cout<<line<<"\n";
        cout<<"EXITING PROGRAM\n";
        cout<<"Notes for SYNTAX:\n";
        cout<<"1) All lines should be between 21 and 80 columns (inclusive) in length.\n";
        cout<<"2) 10 columns for LABEL, 10 columns for OPCODE and 60 columns for OPERAND.\n";
        cout<<"3) BYTE operand should be inside X\'\' or C\'\'\n";
        exit(0);
    }
    else if(type == 2){
        cout<<"UNDEFINED SYMBOL IN THE FOLLOWING LINE\n";
        cout<<line<<"\n";
        cout<<"EXITING PROGRAM\n";
        exit(0);
    }
    else if(type == 3){
        cout<<"INVALID OPERATION CODE IN THE FOLLOWING LINE\n";
        cout<<line<<"\n";
        cout<<"EXITING PROGRAM\n";
        exit(0);
    }
    else if(type == 4){
        cout<<"COMMA WARNING IN THE FOLLOWING LINE\n";
        cout<<line<<"\n";
        cout<<"In order to fix it, do indexed addressing as follows: OPCODE <SYMBOL>,X\n";
    }
    else if(type == 5){
        cout<<"DUPLICATE SYMBOL IN THE FOLLOWING LINE\n";
        cout<<line<<"\n";
        cout<<"EXITING PROGRAM\n";
        exit(0);
    }
    else if(type == 6){
        cout<<"ILLEGAL EXTDEF IN THE FOLLOWING LINE\n";
        cout<<line<<"\n";
        cout<<"EXITING PROGRAM\n";
        exit(0);
    }
    else if(type == 7){
        cout<<"DISPLACEMENT OUT OF BOUNDS IN THE FOLLOWING LINE\n";
        cout<<line<<"\n";
        cout<<"EXITING PROGRAM\n";
        exit(0);
    }
    else if(type == 8){
        cout<<"LITTAB VALUES NOT DEFINED\n";
        cout<<"EXITING PROGRAM\n";
        cout<<"PLEASE USE LTORG BEFORE CSECT\n";
        exit(0);
    }
}

//returns true if input character is an operator
bool is_operator(char c){
    if(c == '*')
        return true;
    if(c == '/')
        return true;
    if(c == '+')
        return true;
    if(c == '-')
        return true;
    return false;
}

//returns true if input string is an operator
bool is_operator(string s){
    if(s == "*")
        return true;
    if(s == "/")
        return true;
    if(s == "+")
        return true;
    if(s == "-")
        return true;
    return false;
}

//stores postfix form of expression
vector<string> postFix;
//used in converting expression to its postfix form
stack<char> st;
string temp;

//function which clears global variables at the start of function
void clear_variables(){
    postFix.clear();
    while(!st.empty()){
        st.pop();
    }
}

//function which adds input string to postfix expression
//and then erases its contents
void add_string(string& s){
    postFix.push_back(s);
    s.clear();
}

//add character to stack
void add_char(char c){
    st.push(c);
}

//Evaluate expressions for EQU, WORD and BASE.
//In the case of WORD, EXTREF symbols can also be used.
//Not so in the case of BASE and EQU
int evaluateExpression(string s,bool sameProgramFlag,int recordStartAddress, int len){
	//map of character precedence
	map<char,int> precedence;
	precedence['*'] = 1;
	precedence['/'] = 1;
	precedence['+'] = 0;
	precedence['-'] = 0;

    clear_variables();

    //First converting to postfix
	int leng = s.length();
	for(int i=0;i<leng;i++){
		if(!is_operator(s[i])){
			temp.push_back(s[i]);
		}
		else {
			add_string(temp);
			while(st.size()>0 && precedence[s[i]]<precedence[st.top()]){
				char c=st.top();
				st.pop();
				temp.push_back(c);
				add_string(temp);
			}
			add_char(s[i]);
		}
	}
	if(temp.size() > 0){
		add_string(temp);
	}
	while(st.size() > 0){
		char c=st.top();
		temp.push_back(c);
		st.pop();
		add_string(temp);
	}
	//Evaluate postfix
	stack<string> st2;
	for(int i=0;i<(int)postFix.size();i++){
		if(!is_operator(postFix[i])){
			temp=postFix[i];
			st2.push(temp);
		} else {
			string t2=st2.top();
			st2.pop();
			string t1=st2.top();
			st2.pop();
			int x=0, y=0;
			//To check if they are in the same program or should be externally fetched.
			bool xFound=false, yFound=false;
			//If t1 is a declared symbol
			if(SYMTAB.count(t1)){
                //same program
				if(SYMTAB[t1].count(progName)){
					xFound=true;
					string hex_num = SYMTAB[t1][progName];
					x=hex_to_dec(hex_num);
				} else if(SYMTAB[t1].count("PREDEFINED")){
				    //Register values
					xFound=true;
					x=hex_to_dec(SYMTAB[t1]["PREDEFINED"]);
				} else if(EXTREF_SYMBOLS.find(t1)==EXTREF_SYMBOLS.end()){
					//Not present in EXTERNAL REFERENCE symbols.
					//Current program cannot access.
					UNDEFINED_SYMBOL=true;
					return -1;
				}
			} else if(isInteger(t1)){
			    //if it is an integer
				xFound=true;
				x=stoi(t1);
			} else {
			    //It hasn't been declared anywhere nor is it an integer.
				UNDEFINED_SYMBOL=true;
				return -1;
			}

            //If t2 is a declared symbol
			if(SYMTAB.count(t2)){
			    //same program
				if(SYMTAB[t2].count(progName)){
					yFound=true;
					y=hex_to_dec(SYMTAB[t2][progName]);
				} else if(SYMTAB[t2].count("PREDEFINED")){
				    //Register values
					yFound=true;
					y=hex_to_dec(SYMTAB[t2]["PREDEFINED"]);
				} else if(EXTREF_SYMBOLS.find(t2)==EXTREF_SYMBOLS.end()){
					//Not present in EXTERNAL REFERENCE symbols.
					//Current program cannot access.
					UNDEFINED_SYMBOL=true;
					return -1;
				}
			} else if(isInteger(t2)){
			    //if it is an integer
				yFound=true;
				y=stoi(t2);
			} else {
			    //It hasn't been declared anywhere nor is it an integer.
				UNDEFINED_SYMBOL=true;
				return -1;
			}
            //For EQU and BASE, the labels should be declared in the same section.
            //Therefore a flag is passed to indicate this.
			if(sameProgramFlag && (!xFound || !yFound)){
				UNDEFINED_SYMBOL=true;
				return -1;
			}

			if(postFix[i][0] == '+'){
                st2.push(to_string(x+y));
                //if symbol is not found, make modification records
                if(!xFound){
                    string modifRecord = "M";
                    string modifRecordStartAddress=dec_to_hex(recordStartAddress);
                    modifRecordStartAddress = make_len(modifRecordStartAddress, 6, 0, '0');
                    string modifRecordLen=dec_to_hex(len);
                    modifRecordLen = make_len(modifRecordLen, 2, 0, '0');
                    string str = modifRecordStartAddress+modifRecordLen+"+"+t1;
                    modifRecord.append(str);
                    MODIF_RECORDS.push_back(modifRecord);
                }
                if(!yFound){
                    string modifRecord = "M";
                    string modifRecordStartAddress=dec_to_hex(recordStartAddress);
                    modifRecordStartAddress = make_len(modifRecordStartAddress, 6, 0, '0');
                    string modifRecordLen=dec_to_hex(len);
                    modifRecordLen = make_len(modifRecordLen, 2, 0, '0');
                    string str = modifRecordStartAddress+modifRecordLen+"+"+t2;
                    modifRecord.append(str);
                    MODIF_RECORDS.push_back(modifRecord);
                }
            }
            else if(postFix[i][0] == '-'){
                st2.push(to_string(x-y));
                //if symbol is not found, make modification records
                if(!xFound){
                    string modifRecord = "M";
                    string modifRecordStartAddress=dec_to_hex(recordStartAddress);
                    modifRecordStartAddress = make_len(modifRecordStartAddress, 6, 0, '0');
                    string modifRecordLen=dec_to_hex(len);
                    modifRecordLen = make_len(modifRecordLen, 2, 0, '0');
                    string str = modifRecordStartAddress+modifRecordLen+"+"+t1;
                    modifRecord.append(str);
                    MODIF_RECORDS.push_back(modifRecord);
                }
                if(!yFound){
                    string modifRecord = "M";
                    string modifRecordStartAddress=dec_to_hex(recordStartAddress);
                    modifRecordStartAddress = make_len(modifRecordStartAddress, 6, 0, '0');
                    string modifRecordLen=dec_to_hex(len);
                    modifRecordLen = make_len(modifRecordLen, 2, 0, '0');
                    string str = modifRecordStartAddress+modifRecordLen+"-"+t2;
                    modifRecord.append(str);
                    MODIF_RECORDS.push_back(modifRecord);
                }
            }
            else if(postFix[i][0] == '*'){
                st2.push(to_string(x*y));
                //EXTREF is not allowed here
                if(!xFound || !yFound){
                    SYNTAX_ERROR=true;
                    return -1;
                }
            }
            else if(postFix[i][0] == '/'){
                st2.push(to_string(x/y));
                //EXTREF is not allowed here
                if(!xFound || !yFound){
                    SYNTAX_ERROR=true;
                    return -1;
                }
            }
		}
	}
    //if the top of st2 contains an integer
	if(isInteger(st2.top())){
		int num = stoi(st2.top());
		return num;
	} else {
		//if the value is a symbol
		if(SYMTAB.count(st2.top())==0){
			//If not in SYMTAB, UNDEFINED SYMBOL
			UNDEFINED_SYMBOL=true;
			return -1;
		} else if(SYMTAB[st2.top()].count(progName)){
			//If it is in SYMTAB and in the current section
			return hex_to_dec(SYMTAB[st2.top()][progName]);
		} else if(EXTREF_SYMBOLS.count(st2.top())){
			//If it is in EXTREF SYMBOLS
			if(sameProgramFlag){
				//FOR BASE and EQU, this isn't allowed
				UNDEFINED_SYMBOL=true;
				return -1;
			}
			string modifRecord = "M";
            string modifRecordStartAddress=dec_to_hex(recordStartAddress);
            modifRecordStartAddress = make_len(modifRecordStartAddress, 6, 0, '0');
            string modifRecordLen=dec_to_hex(len);
            modifRecordLen = make_len(modifRecordLen, 2, 0, '0');
            string str = modifRecordStartAddress+modifRecordLen;
            modifRecord.append(str);
            modifRecord.append("+");
            modifRecord.append(st2.top());
            MODIF_RECORDS.push_back(modifRecord);
			return 0;
		} else {
			//it is outside the scope of current program
			UNDEFINED_SYMBOL=true;
			return -1;
		}
	}

}

//function for initializing global variables
void init(){
    LOCCTR=0;
    start_address=0;
    programLength=0;
    label="";
    opcode="";
    operand="";
    progName="";
    extendedFlag=false;
    SYNTAX_ERROR=false;
    UNDEFINED_SYMBOL=false;
    input_filename = "assembler_input.txt";
}

//handles cases where filename of input coincides
//with filename of intermediate file and output file
void handle_collision(string& filename){
    if(filename == "assembler_intermediate.txt" || filename == "assembler_output.txt")
    {
        cout<<"Invalid filename\n";
        cout<<"Kindly specify filename other than \"assembler_intermediate.txt\" and \"assembler_output.txt\"\n";
        cout<<"EXITING PROGRAM\n";
        exit(0);
    }
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

//checks if input line is a comment
bool isCOMMENT(){
    if(word_list[0][0] == '.')
        return true;
    return false;
}

//writes given string to output file
void output_line(string &line){
    fout<<line<<endl;
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
        if(!checkEMPTY())
            break;
    }
    return;
}

//handles comments at file beginning
void handle_comments_at_beginning_of_file(int pass){
    while(!fin.eof() && isCOMMENT()){
        if(pass == 1){
            line = "              " + line;
            output_line(line);
        }
        input_line();
        generate_wordlist();
    }
}

//returns true if both strings are equal
//returns false otherwise
bool isEQUAL(string a, string b){
    if(a == b)
        return true;
    else return false;
}


int main(int argc, char* argv[]){

    //Initializing global variables
    init();

    //if input filename has been provided
	if(argc >= 2){
        input_filename = argv[1];
        handle_collision(input_filename);
        cout<<"Using specified input filename: "<<input_filename<<"\n";
    }
    //if input filename hasn't been provided
    else{
        cout<<"Filename of input file not specified\n";
        cout<<"Using default input filename: "<<input_filename<<"\n";
    }

    //opening opcodes.txt
    fin.open("opcodes.txt");
    if(!fin){
        handle_file_error(1, "opcodes.txt");
    }
    //populating Operation Code Table
    fill_optab();
    //closing file stream
    fin.close();

    //opening opcodes.txt
    fin.open("symbols.txt");
    if(!fin){
        handle_file_error(1, "symbols.txt");
    }
    //populating Symbol Table
    fill_symtab();
    //closing file stream
    fin.close();

	//Open file passed as argument.
	fin.open(input_filename);
	if(!fin){
        handle_file_error(1, input_filename);
	}
	//Intermediate file
	fout.open("assembler_intermediate.txt");
	if(!fout){
        handle_file_error(2, "assembler_intermediate.txt");
	}

    //PASS 1 BEGINS
	while(getline(fin,line,'\n')){
		//If line is empty, skip it.
		if(stripString(line).empty()){
			continue;
		}
		//If line is a comment, print it.
		else if(stripString(line)[0]=='.'){
		    string line_out = "              " + line;
			output_line(line_out);
			continue;
		}

		//Extract opcode, operand and label from line. If line is invalid, skip it.
		if(!disectLine(line,label,opcode,operand,extendedFlag)){
			if(stripString(line).length()!=0){
                //If input format is not valid
				handle_errors(1, line);
			}
			continue;
		}
		//If the current line contains an END instruction
		if(opcode=="END"){
			string hexLOCCTR=dec_to_hex(LOCCTR);
			hexLOCCTR = make_len(hexLOCCTR, 6, 0, '0');
			fout<<hexLOCCTR<<" 0      "<<line<<'\n';
			break;
		}

        //If the current line contains a START instruction
		//Extract program name and start_address (0 by default)
		if(opcode=="START"){
			progName=label;
			start_address=hex_to_dec(operand);
			operand = make_len(operand, 6, 0, '0');
			LOCCTR=start_address;
			fout<<operand<<" 0      "<<line<<'\n';
			continue;
		}

        //CSECT denotes the start of a new section
		if(opcode=="CSECT"){
            //Inserting old program name entry in map
			progNames[progName]=LOCCTR-start_address;
			LOCCTR=0;
            //New start address
			start_address=0;
			//New program name
			progName=label;
			string empty_string = "";
			output_line(empty_string);
			//write CSECT line to intermediate file
			string line_out = "000000 0      " + line;
			output_line(line_out);
			map<string, string>::iterator itr;
			for(itr = LITTAB.begin();itr!=LITTAB.end();itr++){
                //If any literal from the previous program hasn't been assigned a value
                //show an error message
				if(itr->second=="00000000"){
					handle_errors(8);
				}
			}
		}

		//Hexadecimal value of LOCCTR
		string hexLOCCTR=dec_to_hex(LOCCTR);
		hexLOCCTR = make_len(hexLOCCTR, 6, 0, '0');

		//If line is not comment
		if(stripString(line)[0]!='.'){
			//If there is a label, add that label to SYMTAB. If duplicate, set error flag.
			if(label.length() > 0){
				if(SYMTAB.count(label) == 0){
					SYMTAB[label][progName]=dec_to_hex(LOCCTR);
				} else {
					if(SYMTAB[label].count(progName) || SYMTAB[label].count("PREDEFINED")){
						//it is a duplicate symbol
						//display an error message
						handle_errors(5, line);
					} else {
						if(isEQUAL("EQU",opcode)){
							if(isEQUAL("*", operand)){
                                //the value of LOCCTR has to be used
								SYMTAB[label][progName]=dec_to_hex(LOCCTR);
							} else {
							    //only symbols within the current section are relevant.
								int expressionValue=evaluateExpression(operand,true,0,0);
								//if function encountered some error
								if(expressionValue==-1){
								    //if it is syntax error
									if(SYNTAX_ERROR){
										handle_errors(1, line);
									}
                                    //if it is undefined symbol error
									if(UNDEFINED_SYMBOL){
										handle_errors(2, line);
									}
								}
								SYMTAB[label][progName]=dec_to_hex(expressionValue);
							}
						} else {
						    //store the address otherwise
							SYMTAB[label][progName]=dec_to_hex(LOCCTR);
						}
					}
				}
			}
			if(OPTAB.count(opcode)){
				if(isEQUAL("COMPR",opcode) || isEQUAL("CLEAR",opcode) || isEQUAL("TIXR",opcode)){
                    //instruction format 2
					LOCCTR+=2;
					fout<<hexLOCCTR;
					fout<<' ';
					fout<<'2';
					for(int i=0;i<6;i++)
                        fout<<' ';
				} else {
					if(extendedFlag){
					    //instruction format 4
						LOCCTR+=4;
						fout<<hexLOCCTR;
                        fout<<' ';
                        fout<<'4';
                        for(int i=0;i<6;i++)
                            fout<<' ';
					} else {
					    //instruction format 3
						LOCCTR+=3;
						fout<<hexLOCCTR;
                        fout<<' ';
                        fout<<'3';
                        for(int i=0;i<6;i++)
                            fout<<' ';
					}
				}
                //literal
				if(operand[0]=='='){
					if(LITTAB.count(operand) == 0){
					    //mapping it to default value (00000000)
						LITTAB[operand]="00000000";
					}
				}

			} else {
				if(isEQUAL("WORD",opcode)){
                    //A word takes up 3 bytes
					LOCCTR+=3;
					fout<<hexLOCCTR;
                    fout<<' ';
                    fout<<'3';
                    for(int i=0;i<6;i++)
                        fout<<' ';
				}
				else if(isEQUAL("RESW", opcode)){
				    //A word takes up 3 bytes
                    int num = 3*stoi(operand);
					LOCCTR+=num;
					string lengthOfOperand = to_string(num);
					lengthOfOperand = make_len(lengthOfOperand, 7, 1, ' ');
					fout<<hexLOCCTR;
					fout<<' ';
					fout<<lengthOfOperand;
				}
				else if(isEQUAL("RESB", opcode)){
                    int num = stoi(operand);
					LOCCTR+=num;
					string lengthOfOperand = to_string(num);
					lengthOfOperand = make_len(lengthOfOperand, 7, 1, ' ');
					fout<<hexLOCCTR;
					fout<<' ';
					fout<<lengthOfOperand;
				}
				else if(isEQUAL("BYTE", opcode)){
					//Minimum 3 characters (C'' or X'')
					if(operand.size()<3){
						//display syntax error
						handle_errors(1, line);
					}
					//if operand is hexadecimal string
                    else if(operand[0]=='X' && operand[1]=='\'' && operand.back()=='\''){
						int intermediate=operand.length()-3;
						//cannot have odd length
						//apply a 0 at the end to make the length even
						//display a message to the user showing the changes
						if(intermediate&1){
							cout<<"HEXADECIMAL BYTE OPERAND HAS ODD LENGTH. PADDING WITH 0.\n";
							operand.pop_back();
							operand+="0\'";
							intermediate++;
							line=createLine(label,opcode,operand);
						}
						int num = intermediate>>1;
                        LOCCTR+=num;
                        string lengthOfOperand = to_string(num);
						lengthOfOperand = make_len(lengthOfOperand, 7, 1, ' ');
						fout<<hexLOCCTR;
						fout<<' ';
						fout<<lengthOfOperand;
						intermediate = num;
					}
					//if operand is character string
					else if(operand[0]=='C' && operand[1]=='\'' && operand.back()=='\''){
						//Length of the character string is its size in bytes
						int num = operand.length()-3;
						LOCCTR+=num;
						string lengthOfOperand = to_string(num);
						lengthOfOperand = make_len(lengthOfOperand, 7, 1, ' ');
						fout<<hexLOCCTR;
						fout<<' ';
						fout<<lengthOfOperand;
					} else {
					    //First character should either be C or X.
					    //display syntax error
						handle_errors(1, line);
					}
				} else if(isEQUAL("LTORG", opcode)){
					string line_out = hexLOCCTR + " 0      " + line;
					output_line(line_out);
					//Assign addresses to all Literals in LITTAB
					map<string, string>::iterator itr;
					for(itr = LITTAB.begin(); itr!= LITTAB.end(); itr++){
						if(!isEQUAL("00000000", itr->second)){
							continue;
						}
						operand=itr->first;
						reverse(operand.begin(), operand.end());
						operand.pop_back();
						reverse(operand.begin(), operand.end());
						if(operand.length() <= 2){
                            //Minimum characters=3 (C'' or X'')
                            //display syntax error
							handle_errors(1, line);
						} else if(operand[0]=='X' && operand[1]=='\'' && operand.back()=='\''){
							int intermediate=operand.length()-3;
                            //cannot have odd length
                            //apply a 0 at the end to make the length even
                            //display a message to the user showing the changes
							if(intermediate&1){
								cout<<"HEXADECIMAL BYTE OPERAND HAS ODD LENGTH. PADDING WITH 0.\n";
								operand.pop_back();
								operand+="0\'";
								intermediate++;
							}
							//Make an entry in LITTAB
							LITTAB[itr->first]=dec_to_hex(LOCCTR);
							line=createLine("*",itr->first,"");
							int num = intermediate/2;
							string hex=dec_to_hex(LOCCTR);
							hex = make_len(hex, 6, 0, '0');
							string len=to_string(num);
							len = make_len(len, 7, 1, ' ');
							string line_out = hex + " " + len + line;
							output_line(line_out);
							LOCCTR+=num;
							intermediate = num;
						} else if(operand[0]=='C' && operand[1]=='\'' && operand.back()=='\''){
						    //Make an entry in LITTAB
							LITTAB[itr->first]=dec_to_hex(LOCCTR);
							line=createLine("*",itr->first,"");
							string hex=dec_to_hex(LOCCTR);
							hex = make_len(hex, 6, 0, '0');
							int num = operand.length()-3;
							LOCCTR+=num;
							string len=to_string(num);
							len = make_len(len, 7, 1, ' ');
							string line_out = hex + " " + len + line;
							output_line(line_out);
						} else {
							//First character should either be C or X.
							//display syntax error
							handle_errors(1, line);
						}
					}
					continue;
				} else if(isEQUAL("EQU", opcode) || isEQUAL("EXTREF", opcode) ||
                            isEQUAL("EXTDEF", opcode) || isEQUAL("BASE", opcode)){
					fout<<hexLOCCTR<<" 0      ";
				} else if(isEQUAL("CSECT", opcode)){
                    continue;
                } else {
                    //opcode is not known
					handle_errors(3, line);
				}
			}
		}
		//If comment, remove leading and trailing spaces.
		else
            line=stripString(line);
		output_line(line);
	}

    //Literal pool at the end of the program
    map<string, string>::iterator itr;
	for(itr = LITTAB.begin(); itr!=LITTAB.end(); itr++){
		if(!isEQUAL("00000000", itr->second)){
			continue;
		}
		operand=itr->first;
		reverse(operand.begin(), operand.end());
		operand.pop_back();
		reverse(operand.begin(), operand.end());
		if(operand.length() <= 2){
			//Minimum 3 characters (C'' or X'')
			//display syntax error
			handle_errors(1, line);
		} else if(operand[0]=='X' && operand[1]=='\'' && operand.back()=='\''){
			int intermediate=operand.length()-3;
			//cannot have odd length
            //apply a 0 at the end to make the length even
            //display a message to the user showing the changes
			if(intermediate&1){
				cout<<"HEXADECIMAL BYTE OPERAND HAS ODD LENGTH. PADDING WITH 0.\n";
				operand.pop_back();
				operand+="0\'";
				intermediate++;
			}
			//Make an entry in LITTAB
			LITTAB[itr->first]=dec_to_hex(LOCCTR);
			line=createLine("*",itr->first,"");
			string hex=dec_to_hex(LOCCTR);
			hex = make_len(hex, 6, 0, '0');
			int num = intermediate/2;
			string len=to_string(num);
			len = make_len(len, 7, 1, ' ');
			string line_out = hex + " " + len + line;
			output_line(line_out);
			LOCCTR+=num;
		} else if(operand[0]=='C' && operand[1]=='\'' && operand.back()=='\''){
		    //Make an entry in LITTAB
			LITTAB[itr->first]=dec_to_hex(LOCCTR);
			line=createLine("*",itr->first,"");
			string hex=dec_to_hex(LOCCTR);
			hex = make_len(hex, 6, 0, '0');
			int num = operand.length()-3;
			string len=to_string(num);
			len = make_len(len, 7,  1, ' ');
			string line_out = hex + " " + len + line;
			output_line(line_out);
			LOCCTR+=num;
		} else {
		    //First character should either be C or X.
		    //display syntax error
			handle_errors(1, line);
		}
	}

	progNames[progName]=LOCCTR-start_address;

	//Close input and output file streams
	fin.close();
	fout.close();

	//PASS 1 ENDS

	//PASS 2 BEGINS

	//Read from intermediate file and write object code to output file.
	fin.open("assembler_intermediate.txt");
	if(!fin){
        handle_file_error(1, "assembler_intermediate.txt");
	}
	fout.open("assembler_output.txt");
	if(!fout){
        handle_file_error(2, "assembler_output.txt");
	}

    //Flag used to write end symbol
	bool CSECT_flag=false;
	//Current start address
	int cur_start_addr=0;
	//string for storing current text record
	string textRecord;

    while(getline(fin,line,'\n')){
        //if current line is empty, skip it.
		if(stripString(line).empty()){
			continue;
		}
        //if current line is a comment, skip it.
		if(stripString(line)[0]=='.'){
            continue;
		}
		//address of instruction
		string addr=line.substr(0,6);
		//length of instruction
		string len=line.substr(7,6);
		//instruction
		line=line.substr(14);

		//extract label, opcode, operand, extendedFlag from line
		disectLine(line,label,opcode,operand,extendedFlag);

        //if current line contains START instruction
        //Get the name of first section and the starting address
		if(opcode=="START"){
			progName=label;
			//Create Header record;
			string newProgName=label;
			newProgName = make_len(newProgName, 6, 1, ' ');

			EXTREF_SYMBOLS.clear();
			//set LOCCTR to starting address
			LOCCTR=hex_to_dec(operand);
			cur_start_addr=hex_to_dec(operand);
			start_address=hex_to_dec(operand);

			string startAddressString=operand;
			startAddressString = make_len(startAddressString, 6, 0, '0');
			//length of first section
			int progLength=progNames[progName];
			string progLengthString=dec_to_hex(progLength);
			progLengthString = make_len(progLengthString, 6, 0, '0');
			string line_out = "H" + newProgName + startAddressString + progLengthString;
			output_line(line_out);
		}
		//CSECT denotes the beginning of a new section
		if(opcode=="CSECT"){
            //If text record contains some information, write it to object file
            //then erase its contents
            int current_len = textRecord.length();
			if(current_len){
				string cur_start_addr_string = dec_to_hex(cur_start_addr);
				cur_start_addr_string = make_len(cur_start_addr_string, 6, 0, '0');
				int cur_record_len = LOCCTR-cur_start_addr;
				string record_len=dec_to_hex(cur_record_len);
				record_len = make_len(record_len, 2, 0, '0');
				string line_out = "T" + cur_start_addr_string + record_len + textRecord;
				output_line(line_out);
				textRecord.clear();
			}

            //write modification records to file, if any
            //then erase the records
			for(int i=0; i<(int)MODIF_RECORDS.size(); i++){
				output_line(MODIF_RECORDS[i]);
			}
			MODIF_RECORDS.clear();

            //if previous section was the first section
            //write the END record
			if(CSECT_flag == false){
				string startAddressString=to_string(start_address);
				startAddressString = make_len(startAddressString, 6, 0, '0');
				string line_out = "E" + startAddressString;
				output_line(line_out);
			} else {
			    string line_out = "E";
				output_line(line_out);
			}
			string empty_string = "";
			output_line(empty_string);
			//clear set of EXTREF symbols
			EXTREF_SYMBOLS.clear();
			//set start address and current start address to 0
			start_address=0;
			cur_start_addr=0;
			//set values of progName and newProgName to value contained in label
			progName=label;
			string newProgName=label;
			//set CSECT flag to true
			CSECT_flag=true;
			//set value of LOCCTR to 0
			LOCCTR=0;
			//write header record
			newProgName = make_len(newProgName, 6, 1, ' ');
			int progLength=progNames[label];
			string progLengthString=dec_to_hex(progLength);
			progLengthString = make_len(progLengthString, 6, 0, '0');
			string line_out = "H" + newProgName + "000000" + progLengthString;
			output_line(line_out);
		}

		if(isEQUAL("EXTDEF", opcode)){
            //Identify individual symbols
			vector<string> definitions;
			string temporary="";
			for(int i=0;i<(int)operand.length();i++){
				if(operand[i]==','){
                    definitions.push_back(stripString(temporary));
					temporary.clear();
				} else {
				    temporary.push_back(operand[i]);
				}
			}
			if(temporary.size() > 0){
				definitions.push_back(stripString(temporary));
				temporary.clear();
			}

            //Write Define records
			fout<<'D';
			for(int i=0;i<(int)definitions.size();i++){
				if(SYMTAB.count(definitions[i])==0 || SYMTAB[definitions[i]].count(progName)==0){
					//The symbol must be defined in this program.
					//Display ILLEGAL EXTDEF error
					handle_errors(6, line);
				} else {
				    //Write into Define record
					string temp=definitions[i];
					temp = make_len(temp, 6, 1, ' ');
					string addrString = SYMTAB[temp][progName];
					addrString = make_len(addrString, 6, 0, '0');
					fout<<temp;
					fout<<addrString;
				}
			}
			string empty_string = "";
			output_line(empty_string);
			continue;
		} else if(isEQUAL("EXTREF",opcode)){
		    //Identify individual symbols
			vector<string> definitions;
			string temporary="";
			for(int i=0;i<(int)operand.length();i++){
				if(operand[i]==','){
					definitions.push_back(stripString(temporary));
					temporary.clear();
				} else {
				    temporary.push_back(operand[i]);
				}
			}
			if(temporary.size() > 0){
				definitions.push_back(stripString(temporary));
				temporary.clear();
			}
			//Write Refer records
			fout<<'R';
			for(int i=0;i<(int)definitions.size();i++){
				temporary=definitions[i];
				EXTREF_SYMBOLS.insert(definitions[i]);
				temporary = make_len(temporary, 6, 1, ' ');
				fout<<temporary;
			}
			string empty_string = "";
			output_line(empty_string);
			continue;
		} else if(opcode=="BASE"){
		    //Assists the assembler in base relative displacement computation
			int expressionValue=evaluateExpression(operand,true,0,0);
			//if evaluateExpression() encounters an error
			if(expressionValue==-1){
                //if it is SYNTAX ERROR
				if(SYNTAX_ERROR){
                    //display error message
					handle_errors(1, line);
				}
                //if it is UNDEFINED SYMBOL ERROR
				if(UNDEFINED_SYMBOL){
				    //display error message
                    handle_errors(2, line);
				}
			}
			base_register_value=expressionValue;
		} else if(OPTAB.count(opcode)){
			if(isEQUAL("COMPR",opcode) || isEQUAL("CLEAR",opcode) || isEQUAL("TIXR",opcode)){
                //instruction format 2
				string instruction=OPTAB[opcode];
				vector<string> registers;
				string temporary;
				for(int i=0;i<(int)operand.length();i++){
					if(operand[i]==','){
						registers.push_back(temporary);
						temporary.clear();
					} else {
						temporary.push_back(operand[i]);
					}
				}
				if(temporary.size() > 0){
					registers.push_back(temporary);
				}
				if(isEQUAL("COMPR",opcode) && (registers.size()!=2 || SYMTAB.count(registers[0])==0 \
					|| SYMTAB.count(registers[1])==0 || SYMTAB[registers[0]].count("PREDEFINED")==0 \
					|| SYMTAB[registers[1]].count("PREDEFINED")==0)){
					//COMPR has two operands and both must be predefined registers
					//display SYNTAX ERROR
					handle_errors(1, line);
				} else if(!isEQUAL("COMPR",opcode) && (registers.size()!=1 || SYMTAB.count(registers[0])==0 \
					|| SYMTAB[registers[0]].count("PREDEFINED")==0)){
					//For the other two, one register operand
					//display SYNTAX ERROR
					handle_errors(1, line);
				}
				if(isEQUAL("COMPR",opcode)){
				    //Add the register identification values to the instruction
					instruction+=SYMTAB[registers[0]]["PREDEFINED"] + SYMTAB[registers[1]]["PREDEFINED"];
				} else {
				    //Add the register identification values to the instruction
					instruction+=SYMTAB[registers[0]]["PREDEFINED"] + "0";
				}
				if(hex_to_dec(addr)-cur_start_addr>28){
				    //Write text record
					string cur_start_addr_string = dec_to_hex(cur_start_addr);
					cur_start_addr_string = make_len(cur_start_addr_string, 6, 0, '0');
					string record_len=dec_to_hex(hex_to_dec(addr)-cur_start_addr);
					record_len = make_len(record_len, 2, 0, '0');
					string out_line = "T" + cur_start_addr_string + record_len + textRecord;
					cur_start_addr=hex_to_dec(addr);
					textRecord=instruction;
				} else {
					textRecord.append(instruction);
				}
                //because it is a 2 byte instruction
				LOCCTR+=2;
			} else {
				if(extendedFlag){
					string instruction=OPTAB[opcode];
					//To help with flags
					int integer_instruction=hex_to_dec(instruction)*(1<<24);
					//Extended
					integer_instruction+=(1<<20);
					if(operand[0]=='#'){
                        //Immediate i=1, n=0
						reverse(operand.begin(), operand.end());
						operand.pop_back();
						reverse(operand.begin(), operand.end());
						integer_instruction+=(1<<24);
					} else if(operand[0]=='@'){
					    //Indirect i=0, n=1
						reverse(operand.begin(), operand.end());
						operand.pop_back();
						reverse(operand.begin(), operand.end());
						integer_instruction+=(1<<25);
					} else {
					    //Otherwise i=0 and n=0
						integer_instruction+=(1<<25)+(1<<24);
					}
					string tmp = "";
					for(int i=0;i<(int)operand.length();i++){
						if(operand[i]==','){
						    //Indexed e=1
							integer_instruction+=(1<<23);
							operand = tmp;
							break;
						}
                        else{
                            tmp.push_back(operand[i]);
                        }
					}
					if(SYMTAB.count(operand)){
						if(SYMTAB[operand].count(progName)){
							integer_instruction+=hex_to_dec(SYMTAB[operand][progName]);
						}
                        //if operand is not in EXTREF
                        else if(EXTREF_SYMBOLS.count(operand)==0){
							//display UNDEFINED SYMBOL ERROR
							handle_errors(2, line);
						}
						//if operand is in EXTREF
						else if(EXTREF_SYMBOLS.count(operand)){
                            //Make modification record in new format since symbol is in EXTREF
							string modifRecord="M";
							string modifStartAddress=dec_to_hex(LOCCTR+1);
							modifStartAddress = make_len(modifStartAddress, 6, 0, '0');
							modifRecord+=modifStartAddress+"05+"+operand;
							MODIF_RECORDS.push_back(modifRecord);
						}
					}
					//convert instruction to hexadecimal string
					instruction=dec_to_hex(integer_instruction);
					instruction = make_len(instruction, 8, 0, '0');
					if(hex_to_dec(addr)-cur_start_addr>26){
                        //write previous text record and make a new text record
						string cur_start_addr_string = dec_to_hex(cur_start_addr);
						cur_start_addr_string = make_len(cur_start_addr_string, 6, 0, '0');
						string record_len=dec_to_hex(hex_to_dec(addr)-cur_start_addr);
						record_len = make_len(record_len, 2, 0, '0');
						string line_out = "T" + cur_start_addr_string + record_len + textRecord;
						output_line(line_out);
						textRecord=instruction;
						cur_start_addr=hex_to_dec(addr);
					} else {
					    //append to previous record
						textRecord+=instruction;
					}
					//because it is 4 byte instruction
					LOCCTR+=4;
				} else {
				    //For program counter relative addressing, PC value is required
					int PC_value=hex_to_dec(addr)+stoi(len);
					string instruction = OPTAB[opcode];
					//to help with flags
					int integer_instruction=hex_to_dec(instruction)*(1<<16);
					if(operand.length()){
						if(operand[0]=='#'){
                            //Immediate i=1, n=0
							integer_instruction+=(1<<16);
							reverse(operand.begin(), operand.end());
							operand.pop_back();
							reverse(operand.begin(), operand.end());
						} else if(operand[0]=='@'){
						    //Indirect i=0, n=1
							integer_instruction+=(1<<17);
							reverse(operand.begin(), operand.end());
							operand.pop_back();
							reverse(operand.begin(), operand.end());
						} else {
						    //Otherwise i=1, n=1
							integer_instruction+=(1<<16)+(1<<17);
						}
						string tmp = "";
						for(int i=0;i<(int)operand.length();i++){
						    //Indexed
							if(operand[i]==','){
								integer_instruction+=(1<<15);
								operand=tmp;
								break;
							}
							else tmp.push_back(operand[i]);
						}
					} else {
					    //otherwise i=1, n=1
						integer_instruction+=(1<<16)+(1<<17);
					}

					if(operand.empty()){
                        //for instructions like RSUB
						instruction=dec_to_hex(integer_instruction);
						instruction = make_len(instruction, 6, 0, '0');
					}
					//if operand is integer
					else if(isInteger(operand)){
						integer_instruction+=stoi(operand);
						instruction=dec_to_hex(integer_instruction);
						//while(instruction.length()<6)instruction='0'+instruction;
						instruction = make_len(instruction, 6, 0, '0');
					}
					//if operand is a literal
					else if(LITTAB.count(operand)){
                        //Get the address of the literal
						int value=hex_to_dec(LITTAB[operand]);
						if((value>PC_value && value-PC_value<2047) || (PC_value>value && PC_value-value<2048)){
							//Check for PC relative
							int disp=value-PC_value;
							if(disp<0){
								//Two's complement (12 bit)
								disp=-disp;
								disp=((((~disp)&(0xFFF))+1)&(0xFFF));
							}
							integer_instruction+=disp;
                            //p=1
							integer_instruction+=(1<<13);
						} else if(value>base_register_value && value-base_register_value<4096){
						    //If PC didn't work, try base
							int disp=value-base_register_value;
							integer_instruction+=disp;
							//b=1
							integer_instruction+=(1<<14);
						} else {
							//if none worked, display DISPLACEMENT OUT OF BOUNDS ERROR
							handle_errors(7, line);
						}
						instruction=dec_to_hex(integer_instruction);
						instruction = make_len(instruction, 6, 0, '0');
					}
					//if operand is found in SYMTAB
					else if(SYMTAB.count(operand) && SYMTAB[operand].count(progName)){
						//Attempt with PC
						int value=hex_to_dec(SYMTAB[operand][progName]);
						if((value>=PC_value && value-PC_value<2047) || (PC_value>value && PC_value-value<2048)){
							int disp=value-PC_value;
							if(disp<0){
								//Two's complement
								disp=-disp;
								disp=((((~disp)&(0xFFF))+1)&(0xFFF));
							}
							integer_instruction+=disp;
							//p=1
							integer_instruction+=(1<<13);
						} else if(value>base_register_value && value-base_register_value<4096){
						    //Try base register
							int disp=value-base_register_value;
							integer_instruction+=disp;
							//b=1
							integer_instruction+=(1<<14);
						} else {
							//if none worked, display DISPLACEMENT OUT OF BOUNDS ERROR
							handle_errors(7, line);
						}
						instruction=dec_to_hex(integer_instruction);
						instruction = make_len(instruction, 6, 0, '0');
					} else {
						//In instruction format 3, EXTREF isn't possible
						//display UNDEFINED SYMBOL ERROR
						handle_errors(2, line);
					}

					if(hex_to_dec(addr)-cur_start_addr>27){
                        //write previous record into object file and generate new record
						string cur_start_addr_string = dec_to_hex(cur_start_addr);
						cur_start_addr_string = make_len(cur_start_addr_string, 6, 0, '0');
						string record_len=dec_to_hex(hex_to_dec(addr)-cur_start_addr);
						record_len = make_len(record_len, 2, 0, '0');
						string line_out = "T" + cur_start_addr_string + record_len + textRecord;
						output_line(line_out);
						textRecord=instruction;
						cur_start_addr=hex_to_dec(addr);
					} else {
					    //append into previous record
						textRecord+=instruction;
					}

					LOCCTR+=3;
				}
			}
		} else {
			if(isEQUAL("RESW",opcode) || isEQUAL("RESB",opcode)){
                //compute the required space
				int total;
				if(isEQUAL("RESW",opcode))
                    total = stoi(operand)*3;
                else total = stoi(operand);
				if(textRecord.size() > 0){
				    //write previous record into object file and generate new record
					string cur_start_addr_string = dec_to_hex(cur_start_addr);
					cur_start_addr_string = make_len(cur_start_addr_string, 6, 0, '0');
					string record_len=dec_to_hex(hex_to_dec(addr)-cur_start_addr);
					record_len = make_len(record_len, 2, 0, '0');
					string line_out = "T" + cur_start_addr_string + record_len + textRecord;
					output_line(line_out);
					textRecord.clear();
				}
                //increase the value of LOCCTR by total (required space)
				LOCCTR+=total;
				cur_start_addr=LOCCTR;
			} else if(opcode=="WORD"){
				if(operand.empty()){
					operand="0";
				}
				int expressionValue=evaluateExpression(operand,false,LOCCTR,6);
				//if evaluateExpression() encounters error
				if(expressionValue==-1){
					if(SYNTAX_ERROR){
						//display SYNTAX ERROR
						handle_errors(1, line);
					}
				}
				string instruction = dec_to_hex(expressionValue);
				instruction = make_len(instruction, 6, 0, '0');
				if(hex_to_dec(addr)-cur_start_addr>27){
                    //write record to object file
					string cur_start_addr_string = dec_to_hex(cur_start_addr);
					cur_start_addr_string = make_len(cur_start_addr_string, 6, 0, '0');
					string record_len=dec_to_hex(hex_to_dec(addr)-cur_start_addr);
					record_len = make_len(record_len, 2, 0, '0');
					string line_out = "T" + cur_start_addr_string + record_len + textRecord;
					output_line(line_out);
					textRecord=instruction;
					cur_start_addr=hex_to_dec(addr);
				} else {
					textRecord+=instruction;
				}
				LOCCTR+=instruction.length()/2;
			} else if(isEQUAL("BYTE",opcode) || isEQUAL("*",label)){
				//If it is a literal
				if(isEQUAL("*",label)){
					operand=opcode;
					reverse(operand.begin(), operand.end());
                    operand.pop_back();
                    reverse(operand.begin(), operand.end());
				}
				string instruction=operand;
				if(operand[0]=='X'){
					//If operand is a hexadecimal string, just write the hexadecimal string to file.
					//Remove X'' from string.
					instruction.pop_back();	//Remove last '
                    //Remove X'
					if(instruction.length()>=2)
                    {
                        reverse(instruction.begin(), instruction.end());
                        instruction.pop_back();
                        instruction.pop_back();
                        reverse(instruction.begin(), instruction.end());
                    }
					else instruction.clear();
				} else if(operand[0]=='C'){
					//If operand is a character string, break into hexadecimal parts
					//use getHexBreakup
					instruction.pop_back();
					if(instruction.length()>=2){
						reverse(instruction.begin(), instruction.end());
                        instruction.pop_back();
                        instruction.pop_back();
                        reverse(instruction.begin(), instruction.end());
						instruction=getHexBreakup(instruction);
					}
					else instruction.clear();
				}
				if(hex_to_dec(addr)+instruction.length()/2-cur_start_addr > 30){
                    //write text record to object file
					string cur_start_addr_string = dec_to_hex(cur_start_addr);
					cur_start_addr_string = make_len(cur_start_addr_string, 6, 0, '0');
					string record_len=dec_to_hex(hex_to_dec(addr)-cur_start_addr);
					record_len = make_len(record_len, 2, 0, '0');
					string line_out = "T" + cur_start_addr_string + record_len + textRecord;
					output_line(line_out);
					textRecord=instruction;
					cur_start_addr=hex_to_dec(addr);
				} else {
					textRecord+=instruction;
				}
				//a hexadecimal character is equivalent to a half-byte
				LOCCTR+=instruction.length()/2;
			}
		}
	}

    //write final text record
	if(textRecord.size() > 0){
		string cur_start_addr_string = dec_to_hex(cur_start_addr);
		cur_start_addr_string = make_len(cur_start_addr_string, 6, 0, '0');
		string record_len=dec_to_hex(LOCCTR-cur_start_addr);
		record_len = make_len(record_len, 2, 0, '0');
		string line_out = "T" + cur_start_addr_string + record_len + textRecord;
        output_line(line_out);
		textRecord.clear();
	}
	//write final modification records
	for(int i =0;i<(int)MODIF_RECORDS.size();i++){
		output_line(MODIF_RECORDS[i]);
	}
	MODIF_RECORDS.clear();

    //write END record
	if(!CSECT_flag){
		string startAddressString=to_string(start_address);
		startAddressString = make_len(startAddressString, 6, 0, '0');
		string line_out = "E" + startAddressString;
	    output_line(line_out);
	} else {
	    string line_out = "E";
	    output_line(line_out);
	}

	//PASS 2 ENDS

}
