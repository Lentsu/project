#ifndef _PROJECT_H_
#define _PROJECT_H_

// Includes
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>	// for ptrdiff_t
#include <ctype.h>

// NOTE: _<name> for private variable and typenames 

// Exit codes 
enum {  
	OK = 0,
	CMD_NULL_DATA,
	CMD_BROKEN_HANDLE,
	CMD_ARG0_NOT_CHAR
};

// word type to represent a member of a linked string
typedef struct _Word {
	const char* word;
	struct _Word* next;
} word_t;

/**
 *  Handle structure to hold command label and arguments!!!
**/
typedef struct _CMD_Handle {

	// Command data as a string 
	char* _data;
	// Label of the command
	char label;
	// Tokens in data as a linked list
	word_t* words;
	// Next word
	word_t* next_word;

} cmd_h;

// Function to construct a handle from string input
int read_h(cmd_h*, const char*);

// Function to free the memory used by a handle
void free_h(cmd_h*);



/**
 *  'flag' type to represent boolean values
**/
typedef unsigned char flag;



/**
 * Structure to represent a meeting!!!
**/
typedef struct _Meeting {

	// Description of the meeting 
	char* description;
	// Month represented as an 8 bit unsigned integer value
	uint8_t month;
	// Day represented as an 8 bit unsigned integer value
	uint8_t day;
	// Day represented as an 8 bit unsigned integer value
	uint8_t hour;

} meeting;



#endif //! _PROJECT_H_
