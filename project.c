#include <stdio.h>
#include "project.h"

int main() {

	// SCHEDULE OBJECT that HOLDS MEETINGS as an ORGANIZED linked list
	schedule sched;
	schedule_init(&sched);

	// Flag to handle terminating the program
	flag Q = 0;

	// Buffer to read a line of data into
	char linebuf[1024];

	// Holder for the command label and argument(s) 
	cmd_h handle;

	// Loop until Q flag is set
	while (!Q) {

		// Flag to indicate success
		flag S = 1;

		// Get a line of input from stdin to the buffer
		fgets(linebuf, 1024, stdin);

		// Process the line and read into the handle
		// 	If it work out...
		int read_state = read_h(&handle, linebuf);

		// If error state
		if (read_state)
			continue;

		// If next_word isn't defined, skip processing
		if (!handle.next_word) {
			free_h(&handle);
			continue;
		}
		
		// Determine what to do
		switch(handle.label) {

			case 'A':
				// Try adding to the schedule
				S = schedule_add(&sched, &handle);
				break;

			case 'D':
				// Try deleting from the schedule
				S = schedule_del(&sched, &handle);
				break;

			case 'L':
				// Try listing the schedule
				S = schedule_list(&sched, &handle);
				break;

			case 'W':
				// Try writing the schedule into a file
				S = schedule_write(&sched, &handle);
				break;

			case 'O':
				// Try loading the schedule from a file
				S = schedule_load(&sched, &handle);
				break;

			// Quit
			case 'Q':
				Q = 1;
				break;

			default:
				printf("Invalid command %c\n", handle.label);
				S = 0;	// unset success flag
				break;
		}

		// Free the command handle
		free_h(&handle);

		// If SUCCESS flag is set, print
		if (S)
			printf("SUCCESS\n");

	}

	// Free the schedule
	schedule_free(&sched);

	return 0;
}

/*
 * COMMAND HANDLE FUNCTION IMPLEMENTATIONS
*/

int read_h(cmd_h* h, const char* str) {

	// Duplicate the string into the handle
	// h->_data = strdup(str); // Oh... right
	size_t len = strlen(str);
	h->_data = malloc((len+1)*sizeof(char));
	strcpy(h->_data, str);

	// Allocate the memory for the first argument
	h->words = (word_t*)malloc(sizeof(word_t));
	h->next_word = h->words;
	
	// Pointer to read into
	char* ptr;
	
	// Set the memory to avoid accessing it uninitialized
	memset(h->next_word, 0, sizeof(word_t));

	// Tokenize the string
	ptr = strtok(h->_data, " \n");
	
	// If the first argument is NULL
	if (!ptr) {
		free(h->words);
		free(h->_data);
		return CMD_NULL_DATA;
	}
	
	// Read the first argument (Label)
	h->next_word->word = ptr;
	
	// Check that it's just one character
	if (strlen(ptr) != 1) {
		free(h->words);
		free(h->_data);
		return CMD_ARG0_NOT_CHAR;
	}

	// Set the label
	h->label = h->next_word->word[0];

	// Read the next token
	ptr = strtok(NULL, " \n");

	// While there is a token
	while(ptr) {

		// Allocate space for the next argument
		h->next_word->next = (word_t*)malloc(sizeof(word_t));

		// Move to next memory location
		h->next_word = h->next_word->next;

		// Set the memory to avoid accessing it uninitialized
		memset(h->next_word, 0, sizeof(word_t));

		// Read the next token
		h->next_word->word = ptr;
		ptr = strtok(NULL, " \n");
	}

	// Reset next word
	h->next_word = h->words;

	// Everything ok
	return OK; 
}

void free_h(cmd_h* h) {
	
	// Free the words
	h->next_word = h->words;
	while(h->words) {
		h->next_word = h->words->next;
		free(h->words);
		h->words = h->next_word;
	}

	// Free the data
	free(h->_data);

}

/*
 * SCHEDULE AND MEETING FUNCTION IMPLEMENTATIONS
*/

// Auxillary to read a meeting
static meeting* meeting_read(word_t* w) {

	// Check that there are 4 words, no less, no more
	if (!w || !w->next || !w->next->next || !w->next->next->next || w->next->next->next->next) {
		fprintf(stderr, "meeting_read: wrong number of words.\n");
		return NULL;
	}
	
	/** words
	 * 	w FOR DESCRIPTION
	 *	w->next FOR MONTH
	 *	w->next->next FOR DAY
	 *	w->next->next->next FOR HOUR
	 *	w->next->next->next->next SHOULDN'T EXIST
	**/
	word_t* desc_w = w;
	word_t* mon_w = w->next;
	word_t* day_w = w->next->next;
	word_t* hour_w = w->next->next->next;

	// Allocate memory for the meeting
	meeting* ret = (meeting*)malloc(sizeof(meeting));
	memset(ret, 0, sizeof(meeting));

	// Read the description into ret
	size_t desc_len = strlen(desc_w->word);
	ret->description = (char*)malloc((desc_len+1)*sizeof(char));
	strcpy(ret->description, desc_w->word);

	// Read the time using sscanf magic
	if(!sscanf(mon_w->word, "%" SCNu8, &ret->month)
	  || !sscanf(day_w->word, "%" SCNu8, &ret->day)
	  || !sscanf(hour_w->word, "%" SCNu8, &ret->hour)) {

	  // If there's an error, free the newly allocated space and return NULL
	  fprintf(stderr, "meeting_read: unscannable arguments.\n");
	  free(ret->description);
	  free(ret);
	  return NULL;
	}

	return ret;
}

// Auxillary to free a meeting
static void meeting_free(meeting* m) {
	// Free the description string
	free(m->description);
	// Free the other members 
	free(m);
}

flag schedule_add(schedule* s, cmd_h* h) {
	
	// Point the next word one past the label (first argument)
	h->next_word = h->words->next;

	// Read a meeting from the command handle
	meeting* new = meeting_read(h->next_word);

	// Reset next_word in command handle
	h->next_word = h->words;

	// If meeting can't be read
	if (!new){
		return 0;
	}

	// Add the newly created meeting to the schedule
	return schedule_add_meeting(s, new);

}

flag schedule_add_meeting(schedule* s, meeting* new) {
	
	s->next_meeting = s->meetings;
	while(s->next_meeting->next) {

		// Check that there isn't another meeting in the same time window
		if (s->next_meeting->next->month == new->month
				&& s->next_meeting->next->day == new->day
				&& s->next_meeting->next->hour == new->hour) {
			fprintf(stderr, "add: time already reserved.\n");
			meeting_free(new);
			return 0;
		}

		s->next_meeting = s->next_meeting->next;
	}

	// -> NO DUPLICATES!
	
	// Check that the read date data is realistic
	if (!(new->month > 0 && new->month < 13)
			|| !(new->day > 0 && new->day < 32) || !(new->hour < 24)) { fprintf(stderr, "add: scanned time isn't realistic.\n"); meeting_free(new);
		return 0;
	}
	
	// Reset the next_meeting and make a temporary to hold the old value
	meeting* old_next = s->meetings;	// Points to the null member by default
	s->next_meeting = s->meetings->next;

	// Go through meetings as long as the new meeting has smaller month variable
	while (s->next_meeting && s->next_meeting->month < new->month) {
		old_next = s->next_meeting;
		s->next_meeting = s->next_meeting->next;
	}
	
	// If the meetings have the same month variable
	if (s->next_meeting && new->month == s->next_meeting->month) {

		// Go through meetings as long as the new meeting has smaller day variable
		while (s->next_meeting && s->next_meeting->day < new->day) {
			old_next = s->next_meeting;
			s->next_meeting = s->next_meeting->next;
		}
		// If the meetings have the same day variable
		if (s->next_meeting && new->day == s->next_meeting->day) {

			// Go through meetings as long as the new meeting has smaller hour variable
			while (s->next_meeting && s->next_meeting->hour < new->hour) {
				old_next = s->next_meeting;
				s->next_meeting = s->next_meeting->next;
			}
		}
	}
	// Put the new meeting in between old_next and next_meeting
	old_next->next = new;
	new->next = s->next_meeting;

	// Reset the pointer
	s->next_meeting = s->meetings;

	// Return 1 to indicate success
	return 1;
}

flag schedule_del(schedule* s, cmd_h* h) {

	// Count the arguments (after label)
	size_t arg_count = 0;
	h->next_word = h->words->next;
	while (h->next_word) {
		++arg_count;
		h->next_word = h->next_word->next;
	}
	
	// Reset next_word
	h->next_word = h->words;

	// If not 3
	if (arg_count != 3) {
		fprintf(stderr, "delete: wrong number of arguments.\n");
		return 0;
	}
	
	// Value members
	word_t* mon_w = h->next_word->next;
	word_t* day_w = h->next_word->next->next;
	word_t* hour_w = h->next_word->next->next->next;

	uint8_t del_mon;
	uint8_t del_day;
	uint8_t del_hour;

	// Read the values with sscanf magic
	if (!sscanf(mon_w->word, "%" SCNu8, &del_mon)
		|| !sscanf(day_w->word, "%" SCNu8, &del_day)
		|| !sscanf(hour_w->word, "%" SCNu8, &del_hour)) {
		fprintf(stderr, "delete: unscannable arguments.\n");
		return 0;
	}

	// Flag to tell if the value was deleted or not
	flag deleted = 0;

	// Loop through the linked list of meetings
	s->next_meeting = s->meetings->next;
	meeting* old_next = s->meetings;
	while (s->next_meeting) {

		// If time-values match
		if (s->next_meeting->month == del_mon
				&& s->next_meeting->day == del_day
				&& s->next_meeting->hour == del_hour) {

			// Set old_next to point to the current next
			old_next->next = s->next_meeting->next;
			
			// Free the memory on the current meeting
			meeting_free(s->next_meeting);
			
			// Set the delete flag
			deleted = 1;
			break;
		}

		// Next
		old_next = s->next_meeting;
		s->next_meeting = s->next_meeting->next;
	}
	
	// Reset the next_meeting
	s->next_meeting = s->meetings;

	// If nothing is deleted
	if (!deleted)
		fprintf(stderr, "delete: no meeting at given time.\n");

	return deleted;
}

void schedule_print(FILE* stream, schedule* s) {

	// Loop through all nodes (assuming they are in order)
	// 	(( Except the first null one ))
	s->next_meeting = s->meetings->next;
	while(s->next_meeting) {
		// Print the meeting
		fprintf (stream, "%s %02hhu.%02hhu at %02hhu\n", 
		  s->next_meeting->description,
		  s->next_meeting->day,
		  s->next_meeting->month,
		  s->next_meeting->hour
		);
		// Next meeting
		s->next_meeting = s->next_meeting->next;
	}

	// Reset the next_meeting pointer
	s->next_meeting = s->meetings;
}

flag schedule_list(schedule* s, cmd_h* h) {
	
	// If h->words->next, wrong number of arguments
	if (h->words->next) {
		fprintf(stderr, "list: command shouldn't have arguments.\n");
		return 0;
	}

	// Print the schedule to stdout
	schedule_print(stdout, s);

	return 1;
}

flag schedule_write(schedule* s, cmd_h* h) {
	
	// Count the arguments (after label)
	size_t arg_count = 0;
	h->next_word = h->words->next;
	while (h->next_word) {
		++arg_count;
		h->next_word = h->next_word->next;
	}

	// Reset the next_word
	h->next_word = h->words;

	if (arg_count != 1) {
		fprintf(stderr, "write: wrong number of arguments.\n");
		return 0;
	}

	// Read the filename
	const char* filename = h->words->next->word;

	// Try opening the file for writing
	FILE* file = fopen(filename, "w");

	// Check if there was an error opening the file
	if (!file) {
		fprintf(stderr, "write: cannot open %s for write.\n", filename);
		return 0;
	}

	// Print the schedule into the file
	schedule_print(file, s);

	// Close the file after using it
	fclose(file);

	return 1;
}

flag schedule_load(schedule* s, cmd_h* h) {

	/** TODO:
	 *  0?: Free and reinitialize the schedule???
	 *	1. OPEN FILE (close at the end)
	 *	2. Read line:
	 *		- Tokenize the string in each line
	 *		- Use strtok to read words
	 *  3. Send the words to meeting_read
	 *  4. Add the read meeting to the schedule
	 *  5. Repeat until EOF
	**/
	
	// Count the arguments after label
	size_t arg_count = 0;
	h->next_word = h->words->next;
	while (h->next_word) {
		++arg_count;
		h->next_word = h->next_word->next;
	}

	// Reset the next_word
	h->next_word = h->words;

	if (arg_count != 1) {
		fprintf(stderr, "load: wrong number of arguments.\n");
		return 0;
	}

	// Open the file
	const char* filename = h->words->next->word;

	// Open the file for reading purposes
	FILE* file = fopen(filename, "r");

	// TODO: DO STUFF HERE !!
	

	
	// Close the file
	fclose(file);

	(void)s;
	return 1;
}

void schedule_free(schedule* s) {

	// Loop through meetings and free them
	while (s->meetings) {
		s->next_meeting = s->meetings;
		s->meetings = s->meetings->next;
		meeting_free(s->next_meeting);
	}
}

void schedule_init(schedule* s) {
	// Reserve space for a list head that doesn't contain a value
	s->meetings = (meeting*)malloc(sizeof(meeting));
	s->next_meeting = s->meetings;
	// Set the memory to avoid accessing it uninitialized
	memset(s->next_meeting, 0, sizeof(meeting));
}
