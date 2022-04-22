#include <stdio.h>
#include "project.h"

int main() {

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

			// Add
			case 'A':
				printf("Add\n");
				break;

			// Delete
			case 'D':
				printf("Delete\n");
				break;

			// List
			case 'L':
				printf("List\n");
				break;

			// Write to file
			case 'W':
				printf("Write\n");
				break;

			// Open/Load from file
			case 'O':
				printf("Open\n");
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

		// Free the handle
		free_h(&handle);

		// If SUCCESS flag is set, print
		if (S)
			printf("SUCCESS\n");

	}
	
	return 0;
}

/*
 * AUXILLARY FUNCTION IMPLEMENTATIONS
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
	char* ptr;	// Pointer to read into
	
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
