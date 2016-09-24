#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "datastruct.h"

/**
* Function opens the required file and reads through the whole file
* It saves the frequency of each character in an array on the heap
* Finally, a pointer to this array is returned.
* If file can't be opened or if array couldn't be allocated on Heap,
* 	then NULL is returned. Ensured that this won't cause memory leaks
*/
unsigned long long int* set_freq(char *path, unsigned long long *sizeof_input_file)
{
	FILE *fptr = fopen(path, "r");	// fptr points to the file we need to read
	if(fptr == NULL)
	{				// validity of the file (exists or not/empty or not)	
		fprintf(stderr, "Failed to read %s file\n", path);
		return NULL;
	}

	/**
	* Allocating 256 units of memory is worth it since grading is only on speed
	* A linear array provides best speed for this operation since we don't know hashing
	*/
	unsigned long long int *freq_array = malloc( sizeof(unsigned long long int) * 256 );
	if(freq_array==NULL)		// In this freq_array each index corresponds to it's respective frequency
	{
		fprintf(stderr, "Unable to allocate memory for array successfully\n");
		fclose(fptr);			// file closed before returning to avoid memory leak
		return NULL;		
	}
	int i;
	for(i = 0; i < 256; ++i)	// loop to set all values in array to ZERO frequency
		freq_array[i] = 0;


	char ch = fgetc(fptr);		// we read first character and check condition
	while( !feof(fptr) )
	{
		//printf("%d ", (int)ch);
		++freq_array[(int)ch];		// the character is now only processed
		*sizeof_input_file = *sizeof_input_file + 1;
		ch = fgetc(fptr);		// NEXT character is read now, if EOF (ie -1) won't access unallocated memory now
	}
	fclose(fptr);
	//printf("\n\n");
	return freq_array;			// array is on Heap and is not freed as it will be used then freed in main()
}

/**
*	Recursive function that is responsible for writing the header into the file.
*	To do so, this function traverses in post-fix order.
*	Upon encountering a leaf, a command '1' is written followed by the character stored in the leaf
*	In case of non-leaf nodes, a command of '0' is written. 
*	To write data into a file the byte and pos_in_byte variables are used. values are added into byte until full.
*	When byte is filled, the byte is written and reset along with pos_in_bytes so that process may repeat indefinitely
*/
void write_header(FILE *fWrite, struct leaf* root, char *byte, char *pos_in_byte)	// pos_in_byte goes from 7 to 0
{
	if(root == NULL)	// fWrite cannot be NULL since already checked in parent function
		return;
	if(root->left==NULL && root->right==NULL)	// at a leaf node
	{
		if(*pos_in_byte <= -1)				// condition that checks of byte is full or not
		{									// if byte is full then we write it into file
			fputc(*byte, fWrite);			// and then reset both the byte(=0) and it's position
			*byte = 0; *pos_in_byte = 7;
		}
		*byte = *byte + (char)pow(2,*pos_in_byte);	// write TRUE(1) into byte at my current location
		*pos_in_byte -= 1;						// move into the next location

		char pos_in_char = 7;				// char it points to is root->ch
		while(pos_in_char >= 0)		// loop reads through the leaf character
		{
			if(*pos_in_byte <= -1)	// resets byte and location specifier if byte is full
			{
				fputc(*byte, fWrite);	// the byte is written into file before being reset
				*byte = 0; *pos_in_byte = 7;
			}
			if( (root->ch & (char)pow(2,pos_in_char) ) != 0)	// if character at pointed location is non ZERO, then write into byte
			{
				*byte = *byte + (char)pow(2, *pos_in_byte);	// write is done at bytes position
			}
			*pos_in_byte -= 1;				// here as well as position must be updated even if it a ZERO
			--pos_in_char;							// character position pointer is updated
		}
	}
	else // if LEFT and/or RIGHT child is existant
	{
		write_header(fWrite, root->left, byte, pos_in_byte);	// RECURSIVE CALLS TO FUNCTION ON BOTH CHILDREN
		write_header(fWrite, root->right, byte, pos_in_byte);
		if(*pos_in_byte <= -1)				// condition that checks of byte is full or not
		{									// if byte is full then we write it into file
			fputc(*byte, fWrite);			// and then reset both the byte(=0) and it's position
			*byte = 0; *pos_in_byte = 7;
		}
		*pos_in_byte -= 1;						// move into the next location	// writing a zero is simply moving specifier
	}
}

/**
*	Recursive Function that is responsible for writing the character read into the new file in it's compressed/encoded form
*	parameter description: ch -> character to be written ; str -> str that will follow path of the tree being traversed ;
*						   root -> root of the tree used ; depth -> the current depth of the node, also index of str    ;
*						   *byte -> the byte data that will be finally written to file ; *pos_in_byte -> write location in byte ; 
*						   *fWrite -> file that is being written into
*	Function returns an integer that acts as a flag so that no unwanted interations need to be carried out
*/
int find_and_write_char(char ch, char* str,struct leaf* root, int depth, char *byte, char *pos_in_byte, FILE *fWrite)
{
	if(root==NULL)	// safety check. Hopefully, the line is never executed
	{
		return 0;
	}
	if(root->ch == ch && root->left == NULL && root->right == NULL)	// if it is a leaf node and match of character is found
	{
		char *str_ptr = str;		// pointer to head of 'str' is created to traverse the str <not needed>
		while(*str_ptr != '\0')
		{
			if(*pos_in_byte <= -1)	// resets byte and location specifier if byte is full
			{
				fputc(*byte, fWrite);
				*byte = 0; *pos_in_byte = 7;
			}
			if( *str_ptr == 1)	// if character at pointed location is non ZERO, then write into byte
			{
				*byte = *byte + (char)pow(2, *pos_in_byte);	// write is done at bytes position
			}
			*pos_in_byte -= 1;				// here as well as position must be updated even if it a ZERO
			str_ptr++;				// move to next address in the string
		}
		return 1;	// 1=> found match
	}
	else if(root->left == NULL && root->right == NULL)	// if a leaf node but not matching then simply return
	{
		return 0;
	}
	else
	{
		int status = 0;		// not needed, but is used to save returned value so that action may later be taken
		str[depth] = 1;	str[depth+1] = '\0';	// 1 represents moving right. str is modified accordingly
		status = find_and_write_char(ch, str, root->right, depth+1, byte, pos_in_byte, fWrite);
		if(status == 1)	// node found
		{
			str[depth] = '\0';	// so that that depth level is not considered for output into byte later
			return 1;
		}
		str[depth] = 2;	str[depth+1] = '\0';// 2 represents moving left. str is modified to store this data for later output into file
		status = find_and_write_char(ch, str,  root->left, depth+1, byte, pos_in_byte, fWrite);
		if(status == 1)
		{
			str[depth] = '\0';	// so that that depth level is not considered for output into byte later
			return 1;
		}
		str[depth] = '\0';	// so that that depth level is not considered for output into byte later
		return 0;
	}
}

/**
*	Function that is responsible for opening the file that is to be converted and setting up required variables.
*	Runs until end of the file until all characters have been abbreviated and written in compressed form into the new file
*	A function called find_and_write_char is called. This function basically writes a certain character into the 'byte' variable
*	and in the case where 'byte' gets filled, it writes the 'byte' into the new file before resetting 'byte' and continuing
*/
void read_then_write(char* pathName, FILE *fWrite, struct leaf* root)
{
	FILE *fRead = fopen(pathName, "r");		// file to be converted is opened
	if(fRead==NULL)							// check if file was accessed and opened
	{										// shouldn't throw an error since the same file was already accessed
		fprintf(stderr, "Could Not read file %s.\n", pathName);
		return;
	}
	// str is an array that will be used to follow the pathway inside the tree. 
	// size of 128 is given as the maximum depth of the tree in ASCII text is about 126-127 leaves deep
	char str[128] = {0};	// index of the str is the same as the depth of the tree
							// therefore don't need option for the same
	char byte = 0, pos_in_byte = 7;		// both byte and pos_in_byte are set up so that byte be used to written individual bytes in
	char ch = fgetc(fRead);
	//printf("%d ", (int)ch);
	while(!feof(fRead))		// loop runs until end of the the file
	{
		str[0] = '\0';		// although not needed, for safety, first character is set as a delimiter
		// ch is the character we are to write.
		// str is the array being used to hold the path the tree is traversing 
		find_and_write_char(ch, str, root, 0, &byte, &pos_in_byte, fWrite);	// 0 represents depth of the root
		ch = fgetc(fRead);	// new character is read
		//printf("%d ",(int)ch);
	}
	if(pos_in_byte != 7)	// checks if byte has important data still stored in it
	{
		fputc(byte, fWrite);		// If TRUE then byte is written into file
		byte = 0; pos_in_byte = 7;	// and then reset both the byte(=0) and it's position <not needed>
	}
	fclose(fRead);				// file that was opened has completed it's purpous and hence has been closed
}

/**
*	Fundamentally acts as the parent function that invokes other functions to write into the file
*	It is responsible for creating new file, calling essential functions that write compression header 
* 	and also the data into the file. Finally it is incharge of cleaning up by de-allocating str_cpy and closing the file
*/
void write_to_file(char *str, struct leaf* root, unsigned long long int sizeof_input_file)
{
	// char *str_cpy = malloc( sizeof(char) * (sizeof(str) + 1) );	// char* to hold a copy of the path of the original file
	// strcpy(str_cpy, str);
	FILE *fWrite;
	char *str_big = malloc( sizeof(char) * (strlen(str) + 6) );	// char* to hold a copy of the path of the original file
	if(str_big == NULL)	// str_big was not allocated properly
	{
		// drastic measure to make program run -_-
		fprintf(stderr, "memory for str_big wasn't allocated.\nUsing strcat to define file name.\n");
		fWrite = fopen( strcat(str, ".huff") , "w");
	}
	else
	{
		strcpy(str_big, str);
		strcat(str_big, ".huff");
		fWrite = fopen(str_big, "w");				// New file to write into is opened
		free(str_big);
	}
	if(fWrite == NULL)												// error checking
	{
		fprintf(stderr, "File could not be created.\n");
		return;
	}

	// NEED TO WRITE NUMBER OF CHARACTERS COMPRESSED
	fprintf(fWrite, "%llu",sizeof_input_file);

	// byte is fundamentally our version of a BUFFER the size of a byte that is filled before written into file and cleared
	char byte = 0;												// variables byte and pos_in_byte are used to store the value of an
	char pos_in_byte = 7;										// individual byte and location inside where next bit is to be written

	write_header(fWrite, root, &byte, &pos_in_byte);			// calls function that writes the header into the file
	if(pos_in_byte != 7)										// Checks if byte has any important data in it
	{															
		fputc(byte, fWrite);			// if true then prints the data into new file
		byte = 0; pos_in_byte = 7;		// and then reset both the byte(=0) and it's position although not needed
	}

	// header is successfully being written
	read_then_write(str, fWrite, root);					// calls function that reads the file and then writes values into new one
	// free(str_cpy);
	fclose(fWrite);
}

int main(int argc, char **argv)
{
	if(argc!=2)	// argc must equal 1 and the string attached shall be the name of the file to be compressed
	{
		fprintf(stderr, "Need 1 argument only but was provided %d arguments\n", argc - 1);
		return -1;
	}
	unsigned long long int sizeof_input_file = 0;
	// in PRINT statements can use " %llu "
	unsigned long long int *freq_array = set_freq(argv[1], &sizeof_input_file);	// set_freq returns frequency array afer reading file
	if(freq_array == NULL)
	{
		// encountered a  problem in set_freq function
		fprintf(stderr, "Terminating program\n");
		return -1;
	}
	// CODE TESTED AND SUCCESSFULLY EXECUTING UNTIL HERE
	
	struct charFreq *charFreq_array;
	
	short i = 0, sizeof_charFreq_array = 0;	// this segment calculates space needed by our charFreq structure
	for(i = 0; i < 256; ++i)
	{
		if(freq_array[i] != 0)
			++sizeof_charFreq_array;
	}

	charFreq_array = malloc(sizeof(struct charFreq) * sizeof_charFreq_array);	// memory allocation for struct charFreq array
	if(charFreq_array == NULL)						// memory safety check
	{
		fprintf(stderr, "Error allocating memory for struct charFreq charFreq_array\n");
		return -1;
	}
	short index_charFreq = 0;	// used as the index of the charFreq_array
	for(i = 0; i < 256; ++i)
	{
		if(freq_array[i] != 0)
		{
			charFreq_array[index_charFreq].ch = (char)i;
			charFreq_array[index_charFreq].freq = freq_array[i];
			++index_charFreq;	// increment index of charFreq_array
		}
	}
	free(freq_array);
	// charFreq_print(charFreq_array, sizeof_charFreq_array);	// testing purposes TESTING ONLY
	charFreq_descending_sort(charFreq_array, sizeof_charFreq_array);
//	charFreq_print(charFreq_array, sizeof_charFreq_array);	// testing purposes TESTING ONLY

	struct node *huffTreeStack = create_huffTreeStack(charFreq_array, sizeof_charFreq_array);

	//print_huffTreeStack(huffTreeStack);						// testing purposes	TESTING ONLY
	free(charFreq_array);	// can be deallocated now since it is of no more use
				// sizeof_freqChar_array is still useful since it is now the size of the huffTreeStack

	struct leaf* huffTree = generate_huffTree(huffTreeStack);	// huffTreeStack deleted inside function
	//print_leaf_inorder(huffTree,0); printf("\n");
	if(huffTree == NULL)
	{
		// already printed error message saying cannot generate huff tree
		char *temp = malloc(sizeof(char)*(strlen(argv[1])+6));
		strcpy(temp, argv[1]);
		strcat(temp, ".huff");
		FILE* fWrite = fopen(temp, "w");
		free(temp);
		fprintf(fWrite, "%llu%c",(unsigned long long)0, '0');
		fclose(fWrite);
		fprintf(stderr, "The file read had no content. New empty file created.\n");
		return -1;
	}
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//											COMPRESSION FROM HERE ONWARDS
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	write_to_file(argv[1], huffTree, sizeof_input_file);

	free_leaf(huffTree);	// huffTree is free'd here as it is no longer needed
	return 0;
}
