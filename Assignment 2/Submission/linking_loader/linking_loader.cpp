/*
Assignment 2
Implementation of Programming Languages Lab
Linking Loader

Name:    Vaibhav Kumar Singh
Roll No: 180101086

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
*/

#include<bits/stdc++.h>

using namespace std;

//used for storing pairs of addresses of EXTDEF symbols and section names
map<string,string> ESTAB;

//name of assembler intermediate file (assembler_intermediate.txt by default)
string assembler_intermediate_filename = "assembler_intermediate.txt";

//name of assembler output file (assembler_output.txt by default)
string assembler_output_filename = "assembler_output.txt";

//input file stream
ifstream fin;

//output file stream
ofstream fout;

//First section starting address
int PROGADDR;

//Current section starting address
int CSADDR;

//string for storing a line of input
string line;

//string for storing current section name
string progName;

//length of current section
int progLength;

//starting address of current section
int progStartAddr;

//vector for storing all words in the input line
vector<string> word_list;

//string for storing a word in a line
string word;

//strings for label, opcode and operand respectively
string label;
string opcode;
string operand;

//true for extended format instruction
//false otherwise
bool extendedFlag;

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

//Retrieve file name without extension so output file name can be created
string getFileWithoutExtension(string s){
	size_t found = s.find(".");
	if (found != string::npos)
        return s.substr(0,found);
	else return s;
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

//Extract opcode, label and operand from a line based on the input file format
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
	//extracting label, opcode and operand fields from input based on input file format
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

//function for handling errors in input file
//TYPE 1 - INCORRECT FORMAT
//TYPE 2 - DUPLICATE EXTERNAL SYMBOL
//TYPE 3 - UNDEFINED EXTERNAL SYMBOL
void handle_errors(int type, string line){
    if(type == 1){
        cout<<"INCORRECT FORMAT IN THE FOLLOWING LINE\n";
        cout<<line<<"\n";
        cout<<"EXITING PROGRAM\n";
        exit(0);
    }
    else if(type == 2){
        cout<<"DUPLICATE EXTERNAL SYMBOL IN THE FOLLOWING LINE\n";
        cout<<line<<"\n";
        cout<<"EXITING PROGRAM\n";
        exit(0);
    }
    else if(type == 3){
        cout<<"UNDEFINED EXTERNAL SYMBOL IN THE FOLLOWING LINE\n";
        cout<<line<<"\n";
        cout<<"EXITING PROGRAM\n";
        exit(0);
    }
}

//function for initializing global variables
void init(){
    PROGADDR=0;
    progStartAddr=0;
    progName.clear();
    progLength=0;
	CSADDR=PROGADDR;
}

//function for extracting a substring out of a string and returning it
//if length is not specified, the end of string is taken as the right end of the substring
string extractSubstring(string& str, int start_idx, int len = -1){
    string res = "";
    if(len == -1){
        for(int i = start_idx; i<str.length(); i++){
            res.push_back(str[i]);
        }
    }
    else{
        for(int i = start_idx; i<start_idx + len; i++){
            res.push_back(str[i]);
        }
    }
    return res;
}

//function which fills second string into the first
//the specified starting address works as the offset
void fill_code(string& s, string& c, int start_addr){
    for(int i=0;i<c.size();i++){
        s[start_addr+i]=c[i];
    }
    return;
}

//returns true if a is divisible by b
//returns false otherwise
bool divisible(int a, int b){
    return a==((a/b)*b);
}

//writes given string to output file
void output(string s){
    fout<<s;
}

//writes given character to output file
void output(char c){
    fout<<c;
}

//returns true if both the characters are equal
//returns false otherwise
bool isEQUAL(char a, char b){
    if(a == b)
        return true;
    else return false;
}

//returns true if both the strings are equal
//returns false otherwise
bool isEQUAL(string a, string b){
    if(a == b)
        return true;
    else return false;
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
//with filename of output file memory map file
void handle_collision(string& filename){
    if(filename == "linking_loader_output.txt" || filename == "linking_loader_memory_map.txt")
    {
        cout<<"Invalid filename\n";
        cout<<"Kindly specify filename other than \"linking_loader_output.txt\" and \"linking_loader_memory_map.txt\"\n";
        cout<<"EXITING PROGRAM\n";
        exit(0);
    }
}

int main(int argc, char* argv[]){
	if(argc >= 3){
        assembler_intermediate_filename = argv[1];
        assembler_output_filename = argv[2];
        handle_collision(assembler_intermediate_filename);
        handle_collision(assembler_output_filename);
        cout<<"Using specified assembler intermediate filename: "<<assembler_intermediate_filename<<"\n";
        cout<<"Using specified assembler output filename: "<<assembler_output_filename<<"\n";
    }
    else{
        cout<<"Filenames of assembler intermediate and assembler output files not specified\n";
        cout<<"Using default assembler intermediate filename: "<<assembler_intermediate_filename<<"\n";
        cout<<"Using default assembler output filename: "<<assembler_output_filename<<"\n";
    }

    //Initializing global variables
	init();

    //opening opcodes.txt
	fin.open("opcodes.txt");
    if(!fin){
        handle_file_error(1, "opcodes.txt");
    }
    //populating Operation Code Table
    fill_optab();
    //closing file stream
    fin.close();

    //opening assembler output file
	fin.open(assembler_output_filename);
	if(!fin){
        handle_file_error(1, assembler_output_filename);
	}

    //PASS 1 BEGINS

	while(getline(fin,line,'\n')){
        //If line is empty, skip it.
		if(stripString(line).empty()){
			continue;
		}

        //Remove spaces from the beginning and end of line
		line=stripString(line);

		//it has to be a Header record
		//if it is not, display INCORRECT FORMAT ERROR
		if(!isEQUAL('H', line[0])){
			handle_errors(1, line);
		}

		//length of program
		int CSLTH=hex_to_dec(extractSubstring(line, 13, 6));
		//starting address of program
		progStartAddr=hex_to_dec(extractSubstring(line, 7, 6));
		//Program Name
		progName=stripString(extractSubstring(line, 1, 6));

        //if the program name has been already defined
        //display DUPLICATE EXTERNAL SYMBOL ERROR
		if(ESTAB.count(progName)){
			handle_errors(2, line);
		}

		//Make an entry in ESTAB for the current program
		//map it to the hexadecimal string containing its starting address
		ESTAB[progName]=dec_to_hex(CSADDR);

		while(getline(fin,line,'\n')){
            //If line is empty, skip it.
			if(stripString(line).empty())
				continue;
            //Remove spaces from the beginning and end of line
			line=stripString(line);
			//if END of section
			if(isEQUAL('E',line[0])){
				break;
			}
			//if it is a Define Record
			//add every symbol to ESTAB
			if(isEQUAL('D',line[0])){
				if(line.length() >= 2){
                    //removing the first character
                    //since it is 'D' (denotes Define Record)
					string symbolString=extractSubstring(line, 1);
					for(int i=0;i<symbolString.length();i+=12){
					    //first 6 characters are for symbol
						string symbol=extractSubstring(symbolString, i, 6);
                        //next 6 characters are for value
						string addr=extractSubstring(symbolString, i+6, 6);
						//if symbol already exists in ESTAB
						//display DUPLICATE EXTERNAL SYMBOL ERROR
						if(ESTAB.count(symbol)){
							handle_errors(2, line);
						}
						//add symbol to ESTAB
						ESTAB[symbol]=dec_to_hex(hex_to_dec(addr)+CSADDR);
					}
				}
			}
		}
		//moving onto the next section
		CSADDR+=CSLTH;
	}

    //closing file stream
	fin.close();

	//PASS 1 ENDS

	//Maximum now addr is CSADDR-1
	//Hence, memory size required is CSADDR
	//This makes it divisible by 32
	int memSize=(CSADDR*2/32+(CSADDR*2%32!=0))*32;	//ceil(CSADDR/32)*32. Hexadecimal, therefore *2

	//Simulating memory in the form of a string of size memSize
	//initially, all blocks of memory contain '.' indicating blank space
	string RAM(memSize, '.');

	//PASS 2 BEGINS

    //opening assembler output file
	fin.open(assembler_output_filename);
	if(!fin){
        handle_file_error(1, assembler_output_filename);
	}

    //Execution Starting Address
	int EXECADDR=PROGADDR;
	//Current section Starting Address
	CSADDR=PROGADDR;

	while(getline(fin,line,'\n')){
        //If line is empty, skip it.
		if(stripString(line).empty())
			continue;
        //Remove spaces from the beginning and end of line
		line=stripString(line);
		//Length of current section
		int CSLTH=hex_to_dec(extractSubstring(line, 13, 6));

		//start reading current section
		while(getline(fin,line,'\n')){
            //If line is empty, skip it.
			if(stripString(line).empty())
				continue;
            //Remove spaces from the beginning and end of line
			line=stripString(line);

			//Text Record
			if(isEQUAL('T',line[0])){
                //effective code
				string code=extractSubstring(line, 9);
                //start address to insert code
                //in half-bytes
				int start_address=2*(hex_to_dec(extractSubstring(line, 1, 6))+CSADDR);
				fill_code(RAM, code, start_address);
			}

			//modification record
			else if(isEQUAL('M',line[0])){
                //sign to be used
				char sign=line[9];
                //symbol of modification record
				string symbol=extractSubstring(line, 10);
				//if symbol not found in ESTAB
				if(ESTAB.count(symbol)==0){
					//display UNDEFINED EXTERNAL SYMBOL ERROR
					handle_errors(3, line);
				}
				//value to be added
				int valueToAdd=hex_to_dec(ESTAB[symbol]);
				//memory address to start modification
				//in half-bytes
				int addressForModification=(CSADDR+hex_to_dec(extractSubstring(line, 1, 6)))*2;
				//length of modification
				//in half-bytes
				int lengthOfModification=hex_to_dec(extractSubstring(line, 7, 2));
				//then last lengthOfModification half-bytes will be modified
				if(lengthOfModification&1)
					addressForModification++;
                //the current value at that address
				string valueToModify;
				for(int i=addressForModification;i<addressForModification+lengthOfModification;i++){
					valueToModify+=RAM[i];
				}
				//easier to use
				int integer_helper=hex_to_dec(valueToModify);
				//modify value
				if(isEQUAL('+',sign))
                    integer_helper = integer_helper + valueToAdd;
                else integer_helper = integer_helper - valueToAdd;
				valueToModify=dec_to_hex(integer_helper);
				valueToModify = make_len(valueToModify, lengthOfModification, 0, '0');
				//write back to memory location
				for(int i=addressForModification;i<addressForModification+lengthOfModification;i++){
					RAM[i]=valueToModify[i-addressForModification];
				}
			}
			//these records contain start address of execution
			else if(isEQUAL('E',line[0])){
				if(line.length() >= 2){
					EXECADDR=CSADDR+hex_to_dec(extractSubstring(line, 1));
				}
				break;
			}
		}
		//moving onto the next section
		CSADDR+=CSLTH;
	}
	//closing file stream
	fin.close();

	//PASS 2 ENDS

    //Writing all the opcodes to the output file
	fin.open(assembler_intermediate_filename);
	if(!fin){
        handle_file_error(1, assembler_intermediate_filename);
	}
	fout.open("linking_loader_output.txt");
	if(!fout){
        handle_file_error(2, "linking_loader_output.txt");
	}

    //set starting address of current section to 0
	CSADDR=0;
	while(getline(fin,line,'\n')){
        //If line is empty, skip it.
		if(stripString(line).empty())
			continue;
        //If line is a comment, skip it.
        if(stripString(line)[0]=='.')
            continue;

        //set extendedFlag to false
		extendedFlag=false;
		string instruction_line=extractSubstring(line, 14);
		//Get label, opcode and operand from line
		disectLine(instruction_line,label,opcode,operand,extendedFlag);
		//Used to set section address
		if(isEQUAL("START", opcode) || isEQUAL("CSECT", opcode)){
			CSADDR=hex_to_dec(ESTAB[stripString(extractSubstring(line, 14, 10))]);
			continue;
		}
		if(OPTAB.count(opcode) || isEQUAL("WORD", opcode) || isEQUAL("BYTE", opcode) || isEQUAL('=', opcode[0])){
            //if important instruction, get its address and length and write it to output file
			int address=2*(hex_to_dec(stripString(extractSubstring(line, 0, 6)))+CSADDR);
			int len=2*stoi(stripString(extractSubstring(line, 7, 6)));
			for(int i=address;i<address+len;i++){
				output(RAM[i]);
			}
			output('\n');
		}
	}

    //closing file streams
	fout.close();
	fin.close();

    //opening linking_loader_memory_map.txt
	fout.open("linking_loader_memory_map.txt");
	if(!fout){
        handle_file_error(2, "linking_loader_memory_map.txt");
	}

    //Write memory status to file
	for(int i=0;i<memSize;i++){
		if(divisible(i, 32)){
			if(i)
                output('\n');
            int num = i/2;
			string hexAddr=dec_to_hex(num);
			hexAddr = make_len(hexAddr, 6, 0, '0');
			output(hexAddr);
			output(" ");
		} else if(divisible(i,8)){
			output(" ");
		}
		output(RAM[i]);
	}

    //closing file stream
	fout.close();
}
