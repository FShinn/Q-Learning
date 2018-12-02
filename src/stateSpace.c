
#include <stdio.h>
#include <string.h>


struct linkedList {
	char *name;
	int occurences;
	struct linkedList *next;
} linkedList;

struct stateSpace {
	int height;
	int width;
	struct state **map;
	struct linkedList *goalList;
} cellState;

struct state {
	char *name; // room name
	int ENWS[4]; // East, North, West, South edges (ordered like unit circle)
} state;



int getMapDimensions(FILE *readfile, struct stateSpace *space);
int allocateMapMem(struct stateSpace *space);
int collectStateNames(FILE *readfile, struct stateSpace *space);
char *addUniqueName(struct linkedList **goalList, char *inName);
int connectENWS(struct stateSpace *space);

int pathExists(struct state s1, struct state s2);

int copySSpace(struct stateSpace *dest, struct stateSpace src);

int buildStateSpace(char *filename, struct stateSpace *space) {
	// open the file
	FILE *readfile;
	if ((readfile = fopen(filename, "r")) == NULL) {
		fprintf(stderr,"could not open file \"%s\"\n", filename);
		return -1;
	}
	
	// use file to get map dimensions, store in space
	if (getMapDimensions(readfile, space) < 0) {
		fclose(readfile);
		return -1;
	}
	
	// allocate map memory
	if (allocateMapMem(space) < 0) {
		fclose(readfile);
		return -1;
	}
	
	// make sure goalList is initialized to NULL
	space->goalList = NULL;
	
	// use file to collect state names
	rewind(readfile);
	if (collectStateNames(readfile, space) < 0) {
		fclose(readfile);
		return -1;
	}
	
	// close the file
	fclose(readfile);
	
	// use state names to determine ENWS values (open/closed edges)
	if (connectENWS(space) < 0)
		return -1;
	
	return 0;
}

// reads input file to determine dimensions of grid map and state space
int getMapDimensions(FILE *readfile, struct stateSpace *space) {
	char c;
	// assume first line is colum titles
	space->width = 1; // n+1 values separated by n commas
	while (((c = fgetc(readfile)) != EOF) && (c != '\n'))
		if (c == ',')
			(space->width)++;
	if (space->width < 1) {
		fprintf(stderr,"invalid format for map file\n");
		return -1;
	}
	// count lines after titles
	space->height = 0;
	while ((c = fgetc(readfile)) != EOF)
		if (c == '\n')
			(space->height)++;
	return 0; // no problems
}

// allocates the memory to map field of space
int allocateMapMem(struct stateSpace *space) {
	if ((space->map = malloc(space->height * sizeof(struct state *))) == NULL) {
		fprintf(stderr, "failed to allocate memory to struct stateSpace space->map\n");
		return -1;
	}
	for (int row=0; row < space->height; row++) {
		if ((space->map[row] = malloc(space->width * sizeof(struct state))) == NULL) {
			fprintf(stderr, "failed to allocate memory to struct stateSpace space->map[%d]\n", row);
			return -1;
		}
	}
	return 0;
}

// reads file for names of states (rooms) and stores their location data in map
int collectStateNames(FILE *readfile, struct stateSpace *space) {
	// prepare read buffer
	const int BUFLEN = 256;
	char buf[BUFLEN]; 
	
	// skip first line of file (column headers)
	while ((fgets(buf, BUFLEN, readfile) != NULL) && buf[strlen(buf)-1] != '\n');
	
	// read in a block of text from readfile and parse into statenames in map
	int row = 0;
	int col = 0;
	int buf_bi = 0, buf_fi = 0; // back and front indicies
	while (fgets(buf+buf_fi, BUFLEN-buf_fi, readfile) != NULL) {
		buf_bi = 0;
		while (buf[buf_bi]) {
			while (buf[buf_fi] && (buf[buf_fi] != ',') && (buf[buf_fi] != '\n') && (buf[buf_fi] != '\r') && (buf[buf_fi] != EOF)) {
				buf_fi++;
			}
			
			// name or no-name handling
			if (buf[buf_fi]) { // buf[buf_fi] == ',', '\n', or '\r'
				if (buf_bi == buf_fi) { // no name between name separators ',' or '\n'
					space->map[row][col].name = NULL;
				}
				else { // name between separators
					char temp = buf[buf_fi];
					buf[buf_fi] = 0;
					if ((space->map[row][col].name = addUniqueName(&(space->goalList), buf+buf_bi)) == NULL)
						return -1;
					buf[buf_fi] = temp;
				}
				// update row and col indicies
				if (col++ >= space->width) { // integrity check
					fprintf(stderr, "a line in gridmap file exceeds expected number of column entries (%d:%d)\n", col,space->width);
					return -1;
				}
				if (buf[buf_fi] != ',') { //  buf[buf_fi] == '\n' or '\r'
					buf[buf_fi+1] = 0;
					if (++row > space->height) { // integrity check
						fprintf(stderr, "number of line entries in gridmap file exceeds expected number of rows\n");
						return -1;
					}
					if (col != space->width) { // integrity check
						fprintf(stderr, "a line in gridmap file does not meet expected number of column entries (%d:%d)\n", col,space->width);
						return -1;
					}
					col = 0;
				}
				buf_bi = ++buf_fi; // prepare to inspect next value
			}
			
			// reached end of buf
			if (!buf[buf_fi]) {
 				if (buf_bi == 0) {
 					fprintf(stderr, "room name in gridmap file is too long\n");
 					return -1;
 				}
				// copy incomplete name from end of buf to start of buf
				int cpy_i = 0;
				while (buf_bi < buf_fi) {
					buf[cpy_i++] = buf[buf_bi++];
				}
				buf_fi = cpy_i;
			}
		}
	}
	if (row != space->height) {
		fprintf(stderr, "number of line entries in gridmap file does not meet expected number of rows\n");
		return -1;
	}
	
	return 0;
}

/* returns pointer to name data of found/created linkedList node, or NULL if error
 */
char *addUniqueName(struct linkedList **goalList, char *inName) {
// 	fprintf(stderr, "inName: %s\n", inName);
	struct linkedList *node = *goalList;
	
	// search goalList for inName
	while (node != NULL) {
		if (strcmp(node->name, inName) == 0) {
			node->occurences++;
			return node->name; // found name in linkedList
		}
		node = node->next;
	}
	
	// inName is new and unique to goalList
	struct linkedList *newGoalNode;
	if ((newGoalNode = malloc(sizeof(struct linkedList))) == NULL) {
		fprintf(stderr, "failed to allocate memory to newGoalNode\n");
		return NULL;
	}
	if ((newGoalNode->name = malloc((strlen(inName)+1)*sizeof(char))) == NULL) {
		fprintf(stderr, "failed to allocate memory to newGoalNode->name\n");
		return NULL;
	}
	strcpy(newGoalNode->name, inName);
	newGoalNode->occurences = 0;
	
	// newGoalNode now heads whole list
	newGoalNode->next = *goalList;
	
	// replace handle on goalList with new head
	*goalList = newGoalNode;
	
	return newGoalNode->name;
}

// uses names of rooms to determine interconnections
int connectENWS(struct stateSpace *space) {
	// initialize all directions of all spaces to impassable, -1
	for (int row=0; row<space->height; row++)
		for (int col=0; col<space->width; col++)
			for (int direction=0; direction<4; direction++)
				space->map[row][col].ENWS[direction] = -1;
	
	// connect all states in stateSpace 'space'
	for (int row=0; row<space->height; row++) {
		for (int col=0; col<space->width; col++) {
			if (space->map[row][col].name != NULL) {
				// check and connect north - south
				if ((row < space->height-1) && (space->map[row+1][col].name != NULL)) {
					if (pathExists(space->map[row][col], space->map[row+1][col])) {
						space->map[row][col].ENWS[3] = 0; // open southward path of northern space
						space->map[row+1][col].ENWS[1] = 0; // open northward path of southern space
					}
				}
				// check and connect east - west
				if ((col < space->width-1) && (space->map[row][col+1].name != NULL)) {
					if (pathExists(space->map[row][col], space->map[row][col+1])) {
						space->map[row][col].ENWS[0] = 0; // open eastward path of western space
						space->map[row][col+1].ENWS[2] = 0; // open westward path of eastern space
					}
				}
			}
		}
	}
	
	return 0;
}

/* checks if a path exists between to states by sharing a name
 * s1.name == s2.name is a fast and valid check because
 *  all states which share a name use the same reference for that name
 * strstr functions are for states with names like 491&450
 *  which have a different name but bridge states named 491 and 450
 */
int pathExists(struct state s1, struct state s2) {
	return (s1.name == s2.name) || strstr(s1.name, s2.name) || strstr(s2.name, s1.name);
}



// creates a unique version of sSpace for each generalized goal state
int buildQTables(struct stateSpace QTables[], struct stateSpace sSpace, int genGoalCount) {
	for (int g=0; g<genGoalCount; g++)
		if (copySSpace(QTables+g, sSpace) < 0)
			return -1;
	return 0;
}

// used for creating QTables and RTables
int copySSpace(struct stateSpace *dest, struct stateSpace src) {
	// sets dimensions of new stateSpace
	dest->height = src.height;
	dest->width = src.width;
	// allocate memory for new stateSpace
	if (allocateMapMem(dest) < 0)
		return -1;
	// copy names and edge values from src to dest
	for (int row=0; row<dest->height; row++)
		for (int col=0; col<dest->width; col++)
			memcpy(&(dest->map[row][col]), &(src.map[row][col]), sizeof(struct state));
	return 0;
}




int isConnected(struct state s) {
	for (int i=0; i<4; i++)
		if (s.ENWS[i] >= 0)
			return 1;
	return 0;
}

int printMap(struct stateSpace space) {
	for (int row=0; row<space.height; row++) {
		for (int col=0; col<space.width; col++) {
			if (space.map[row][col].name == NULL)
				fprintf(stdout, " ");
			else
				fprintf(stdout, "X");
		}
		fprintf(stdout, "\n");
	}
	return 0;
}


void cleanupSSpace(struct stateSpace SSpace) {
	for (int row=0; row < SSpace.height; row++)
		free(SSpace.map[row]);
	free(SSpace.map);
}




