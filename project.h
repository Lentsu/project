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
 *
 *  \param _data is a tokenized string
 *	
 *	\param label is the label character of the command
 *
 *	\param words is a linked list of words pointing to \param _data
 *
 *	\param next_word points to the next word in the linked list
 *
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

	// Next meeting
	struct _Meeting* next;

} meeting;

// Function to read a new meeting from a linked string
static meeting* meeting_read(word_t*);

// Function to free a meeting
static void meeting_free(meeting*);

/**
 * Structure to represent a schedule!!!
 * 
 * 	\param meetings holds a linked list of meetings!
 *
 * 	\param next_meeting points to the list
 *
**/
typedef struct _Schedule {

	// Meetings data as a linked list
	meeting* meetings;
	// Next meeting pointer
	meeting* next_meeting;

} schedule;

// Function to add a meeting to the schedule
flag schedule_add(schedule*, cmd_h*);

// Function to delete a meeting from the schedule
flag schedule_del(schedule*, cmd_h*);

// Function to print the meetings into a stream
void schedule_print(FILE*, schedule*);

// Function to list the meetings into stdout
flag schedule_list(schedule*, cmd_h*);

// Function to write the schedule into a file
flag schedule_write(schedule*, cmd_h*);

// Function to write the schedule into a file
flag schedule_load(schedule*, cmd_h*);

// Function to free the memory used by schedule
void schedule_free(schedule*);

// Function to initialize schedule
void schedule_init(schedule*);

#endif //! _PROJECT_H_
