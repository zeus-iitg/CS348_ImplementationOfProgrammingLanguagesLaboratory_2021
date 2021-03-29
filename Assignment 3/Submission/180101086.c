/*
Assignment 3
Implementation of Programming Languages Lab
Lexical Analyzer
Name:    Vaibhav Kumar Singh
Roll No: 180101086
INSTRUCTIONS FOR RUNNING THE CODE
---------------------------------
1. The environment required for running the code is
					Linux g++
2. Open the terminal and navigate to the directory where
   the contents of this zip file have been extracted.
3. To generate lex.yy.c file, type
				lex 180101086.l
4. To create the executable file, type
				gcc 180101086.c lex.yy.c -o 180101086
5. To run the executable file, type
				./180101086 < input.txt
6. A file named output.txt will be created which will contain
   the output of this program.
*/

//importing necessary header files for program execution
//stdio.h for fprintf
#include <stdio.h>
//stdlib.h for malloc
#include <stdlib.h>
//stdint.h for uint64_t
#include <stdint.h>
//string.h for memset
#include <string.h>

//defining macros for size of hash table and key
//to avoid hardcoding in program
//the optimal size of hash table for murmur hashing
//is 2 times the expected number of elements
//here, I expect 200 elements and hence have set the size to 400
#define size_of_hash_table 400
#define size_of_key 200

//these are defined in lex.yy.c
//hence we use extern keyword here
extern int yylex();
extern int yylineno;
extern char* yytext;

//defining a structure for node of linked list
struct node {
	//will store token specifier (without ^ and #)
	char key[size_of_key];
	//will store token type
	int value;
	//pointer to next node in linked list
	struct node *next;
};

//defining a hash table as an array of linked lists
struct node* hash_table[size_of_hash_table];
//array storing the pointers to last nodes of linked lists
struct node* last_node[size_of_hash_table];

//array storing the size of linked lists for each list
int num_elements[size_of_hash_table];

//creates a new node using given parameters and returns a pointer to it
struct node* create_node(char *key, int value)
{
	struct node *temp = (struct node *)malloc(sizeof(struct node));

	strcpy(temp->key, key);
	temp->value = value;
	temp->next=NULL;

	return temp;
}

//function which uses murmur hashing to compute 32-bit hash of given string
uint32_t MurmurOAAT32(char *key)
{
	uint32_t h = 3323198485ul;
	for (; *key; ++key) {
		h ^= *key;
		h *= 0x5bd1e995;
		h ^= h >> 15;
	}
	return h;
}

//function which uses murmur hashing to compute 64-bit hash of given string
uint64_t MurmurOAAT64(char *key)
{
	uint64_t h = 525201411107845655ull;
	for (; *key; ++key) {
		h ^= *key;
		h *= 0x5bd1e9955bd1e995;
		h ^= h >> 47;
	}
	return h;
}

//function which inserts identifier into hash table (if not already present)
void install_id(char *key, int value, int *list_index, int *hash_index)
{
	//compute hash index of string using murmur hashing
	uint64_t hash = MurmurOAAT64(key);
	*hash_index = hash % size_of_hash_table;

	//setting iterator to head of linked list corresponding to hash_index
	struct node *itr = hash_table[*hash_index];

	//setting current index to 0
	int idx = 0;

	//iterating over the linked list
	while(itr != NULL){
		//if key is already present, set list_index to idx and return
		if(strcmp(itr->key, key) == 0){
			*list_index = idx;
			return;
		}

		itr = itr->next;
		idx++;
	}

	//if control reaches here, it means identifier isn't already present in hash table
	struct node *new_node = create_node(key, value);

	//if linked list is empty
	if(num_elements[*hash_index] == 0){
		//set head pointer and last pointer to the current node
		//and increment the size of linked list
		hash_table[*hash_index] = new_node;
		last_node[*hash_index] = new_node;
		num_elements[*hash_index]++;
	}
	//if linked list isn't empty
	else{
		//set the next of last node to point to current node
		//set current node as last node
		//and increment the size of linked list
		last_node[*hash_index] -> next = new_node;
		last_node[*hash_index] = new_node;
		num_elements[*hash_index]++;
	}

	//since we are adding nodes at the end of linked list
	//list index of added node will be (size of linked list - 1) 
	*list_index = num_elements[*hash_index] - 1;
}

//function which inserts integer into hash table (if not already present)
void install_num(char *key, int value, int *list_index, int *hash_index)
{
	//compute hash index of string using murmur hashing
	uint64_t hash = MurmurOAAT64(key);
	*hash_index = hash % size_of_hash_table;
	
	//setting iterator to head of linked list corresponding to hash_index
	struct node *itr = hash_table[*hash_index];
	
	//setting current index to 0
	int idx = 0;
	
	//iterating over the linked list
	while(itr!=NULL){
		//if key is already present, set list_index to idx and return
		if(strcmp(itr->key, key) == 0){
			*list_index = idx;
			return;
		}

		itr = itr -> next;
		idx++;
	}

	//if control reaches here, it means integer isn't already present in hash table
	struct node *new_node = create_node(key, value);

	//if linked list is empty
	if(num_elements[*hash_index] == 0){
		//set head pointer and last pointer to the current node
		//and increment the size of linked list
		hash_table[*hash_index] = new_node;
		last_node[*hash_index] = new_node;
		num_elements[*hash_index]++;
	}
	//if linked list isn't empty
	else{
		//set the next of last node to point to current node
		//set current node as last node
		//and increment the size of linked list
		last_node[*hash_index] -> next = new_node;
		last_node[*hash_index] = new_node;
		num_elements[*hash_index]++;
	}

	//since we are adding nodes at the end of linked list
	//list index of added node will be (size of linked list - 1) 
	*list_index = num_elements[*hash_index] - 1;
}

int main(){

	//declaring a file stream named fout which will write to output.txt
	FILE *fout = fopen("output.txt", "w");

	//setting value of sizes of all linked lists to 0
	memset(num_elements, 0, sizeof(num_elements));
	
	//stores token type
	int token;
	
	//calling yylex() and saving the returned value in token
	token = yylex();

	//stores the number of the last line that was read
	int last_line = 0;
	
	//for formatting output
	fprintf(fout, "  LINE  TOKEN TYPE         TOKEN SPECIFIER\n");
	fprintf(fout, "  ----  ----------         ---------------\n");

	//loop until the entire file has been read
	while(token){
		//stores hash index for current node
		int hash_index;
		//stores list index for current node
		int list_index;
		//stores current line number	
		int line_num;
		//used for printing line number
		char line_num_str[7];
		line_num_str[6] = '\0';
		//used for printing token type
		char token_type_str[7];
		token_type_str[6] = '\0';

		line_num = yylineno;

		if(line_num != last_line){
			int temp = line_num;
			int cur = 5;
			
			while(temp){
				line_num_str[cur] = '0' + (temp % 10);
				temp /= 10;
				cur--;
			}

			while(cur>=0){
				line_num_str[cur] = ' ';
				cur--;
			}
		}

		else{
			int cur = 5;

			while(cur >= 0){
				line_num_str[cur] = ' ';
				cur--;
			}
		}
		//print line number to file
		fprintf(fout, "%s  ", line_num_str);

		last_line = yylineno;

		int temp = token;
		int cur = 5;
		while(temp){
			token_type_str[cur] = '0' + (temp % 10);
			temp /= 10;
			cur--;
		}

		while(cur >= 0){
			token_type_str[cur] = ' ';
			cur--;
		}
		//print token type to file
		fprintf(fout, "%s    ", token_type_str);		
		
		if(token == 22){
			//stores name of identifier		
			char *var_name = yytext;
			//insert identifier into hash table (if not already present)
			install_id(var_name, 22, &list_index, &hash_index);
			//print token identifier, hash index, list index to file
			fprintf(fout, "^%s (HASH_INDEX: %d, LIST_INDEX: %d)\n", yytext, hash_index, list_index);
		}

		else if(token == 23){
			//stores value of integer
			char *num = yytext;
			//insert integer into hash table (if not already present)
			install_num(num, 23, &list_index, &hash_index);
			//print token identifier, hash index, list index to file
			fprintf(fout, "#%s (HASH_INDEX: %d, LIST_INDEX: %d)\n", yytext, hash_index, list_index);
		}

		else{
			fprintf(fout, "\n");
		}
		//calling yylex() and updating the value of token
		token = yylex();
	}

	fprintf(fout, "\n");

	//printing non-empty entries in hash table to file
	//for formatting output
	fprintf(fout, "      HASH TABLE\n");
	fprintf(fout, "      ----------\n");
	fprintf(fout, "(non-empty entries only)\n\n");
	fprintf(fout, "  INDEX   TOKEN TYPE   TOKEN SPECIFIER\n");
	fprintf(fout, "  -----   ----------   ---------------\n");

	//stores hash index of last node that was processed
	int last_index = -1;
	//used for printing hash index
	char hash_index_str[7];
	hash_index_str[6] = '\0';
	//used for printing token type
	char token_type_str[7];
	token_type_str[6] = '\0';

	//iterate over all linked lists
	for(int i = 0; i < size_of_hash_table; i++){
		//if linked list is non-empty
		if(num_elements[i] > 0){
			//setting iterator to head of linked list corresponding to hash_index
			struct node *itr = hash_table[i];
			//iterating over the linked list
			while(itr != NULL){
				if(last_index != i){
					if(i == 0){
						strcpy(hash_index_str, "     0");
					}
					else{
						int temp = i;
						int cur = 5;
						while(temp){
							hash_index_str[cur] = '0' + (temp%10);
							temp /= 10;
							cur--;
						}
						while(cur >= 0){
							hash_index_str[cur] = ' ';
							cur--;
						}
					}
				}
				else{
					int cur = 5;
					while(cur >= 0){
						hash_index_str[cur] = ' ';
						cur--;
					}
				}
				last_index = i;
				//print hash index to file
				fprintf(fout, "%s    ", hash_index_str);

				int temp = itr->value;
				int cur = 5;
				while(temp){
					token_type_str[cur] = '0' + (temp % 10);
					temp /= 10;
					cur--;
				}

				while(cur >= 0){
					token_type_str[cur] = ' ';
					cur--;
				}

				//print token type to file
				fprintf(fout, "%s          ", token_type_str);
				
				//if it is an identifier, write ^ before its name
				if(itr->value == 22)
					fprintf(fout, "^%s\n",itr->key);
				//if it is an integer, write # before its name
				else if(itr->value == 23)
					fprintf(fout, "#%s\n",itr->key);
				//moving to next node in linked list
				itr=itr->next;
			}
		}
	}
	//closing file stream
	fclose(fout);
	return 0;
}
