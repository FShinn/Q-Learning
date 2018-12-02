/* ***********************************************************************
 * Program: parseArgs.c
 * Description: Fills a struct paramaters with values which determine
 *  Q-Learning paramaters and I/O options for program execution
 * Author: Samuel Shinn
 * Last Modified: 11/12/2017
 * 
 * NOTES:
 *  The function parseArgs(..) is the function called by other files.
 *  The structure of this file is generally that parseArgs(..),
 *   for each of the fields in struct paramater, calls a corresponding
 *   function to determine the field's value.
 *  The functions each search argv for a flag and return the corresponding
 *   input value if supplied, or indicate an error, or return a default 
 *   value if no flag is supplied.
 *  The field char *filename and char *homeroom does not need a flag and 
 *   will each raises an errors if corresponding arguments are not detected.
 * ***********************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>


struct paramaters {
	char *filename;
	char *homeroom;
} paramaters;


char *getNonFlaggedArg(int argc, char** argv, int skips);

int findFlagArg(int argc, char** argv, char c);

void printParams(struct paramaters params);



// set paramaters, use args if supplied
int parseArgs(int argc, char** argv, struct paramaters *params) {
	// filename should be first nonFlaggedArg
	if ((params->filename = getNonFlaggedArg(argc, argv, 0)) == NULL)
		return -1;
	
	// homeroom should be second nonFlaggedArg
	if ((params->homeroom = getNonFlaggedArg(argc, argv, 1)) == NULL)
		return -1;
	
	return 0;
}

// returns char* to 'skips'th non-flagged arg
char *getNonFlaggedArg(int argc, char** argv, int skips) {
	int skipped = 0;
	for (int i=1; i<argc; i++) {
		if (argv[i][0] == '-')
			i++; // skip next arg if arg[i] is a flag
		else if (skipped < skips)
			skipped++; // looking for a further non-flag arg
		else
			return argv[i]; // return address of this arg
	}
	fprintf(stderr, "usage: test GridMapDataFile HomeRoomName\nview readme.txt for paramater flags\n");
	return NULL; // error, arg not found
}

// finds the index of argument containing flag c
int findFlagArg(int argc, char** argv, char c) {
	for (int i=1; i<argc; i++)
		if (argv[i][0] == '-' && argv[i][1] && argv[i][1] == c)
			return i;
	return 0; // search found no flag c
}

void printParams(struct paramaters params) {
	fprintf(stdout, "gridMap: %s\n", params.filename);
	fprintf(stdout, "homeroom: %s\n", params.homeroom);
}





