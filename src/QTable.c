

#include "stateSpace.c"

// should probably move all this queue stuff to another file 
// but running out of time for assignment
struct updateQueue {
	struct pointNode *first;
	struct pointNode *last;
} updateQueue;

struct pointNode {
	int row;
	int col;
	struct pointNode *next;
} pointNode;

int addNode(struct updateQueue *queue, int row, int col) {
	// make a new node to add
	struct pointNode *node;
	if ((node = malloc(sizeof(struct pointNode))) == NULL) {
		perror("malloc");
		return -1;
	}
	
	// set node data
	node->row = row;
	node->col = col;
	node->next = NULL;
	
	// manage queue
	if (queue->first == NULL)
		queue->first = node;
	else
		queue->last->next = node;
	queue->last = node;
	
	return 0;
}

struct pointNode *popNode(struct updateQueue *queue, struct pointNode *node) {
	// queue empty?
	if (queue->first == NULL)
		return NULL;
	
	// store first node contents in paramater supplied node
	node->row = queue->first->row;
	node->col = queue->first->col;
	node->next = NULL;
	
	
	struct pointNode *ref = queue->first;
	queue->first = queue->first->next;
	if (queue->first == NULL)
		queue->last = NULL;
	
	free(ref);
	
	return node;
}



// essential functions
int buildRewardTable(struct stateSpace *RTable, struct stateSpace sSpace, char *goalRoom);
void QLearning(struct stateSpace QTables[], struct stateSpace RTable, struct pointNode target, struct pointNode position);
int pickQTable(struct pointNode pos, struct pointNode target);

// helper functions
int quicksin(int halfPis);
int quickcos(int halfPis);

// debug functions
void printQTables(struct stateSpace QTables[]);
void printQTable(struct stateSpace QTable);


// main file function
int train (struct stateSpace sSpace, struct stateSpace QTables[]) {
	// for each goal room
	struct linkedList *goalNode = sSpace.goalList;
	for (; goalNode != NULL; goalNode = goalNode->next) {
		// find specific target coordinates for goal generalization
		struct pointNode target;
		for (int found=0, row=0; (row<sSpace.height) && (found<=(goalNode->occurences/2)); row++)
			for (int col=0; (col<sSpace.width) && (found<=(goalNode->occurences/2)); col++)
				if (sSpace.map[row][col].name == goalNode->name) { // room names share addresses
					found++;
					target.row = row;
					target.col = col;
				}
		
		// build reward table
		struct stateSpace RTable;
		if (buildRewardTable(&RTable, sSpace, goalNode->name) < 0)
			return -1;
		
		// build update registry (init to all states unregistered)
		int **registry;
		if ((registry = malloc(sSpace.height*sizeof(int *))) ==  NULL) {
			perror("malloc");
			return -1;
		}
		for (int row=0; row < sSpace.height; row++)
			if ((registry[row] = calloc(sSpace.width, sizeof(int))) ==  NULL) {
				perror("calloc");
				return -1;
			}
		
		// build and initialize update queue
		struct updateQueue queue;
		queue.first = NULL;
		for (int row=0; row<sSpace.height; row++)
			for (int col=0; col<sSpace.width; col++)
				if (RTable.map[row][col].name == goalNode->name) { // room names share addresses
					if (addNode(&queue, row, col))
						return -1;
					registry[row][col] = 1; // register spaces in queue
				}
		
		// for each item in queue
		struct pointNode node;
		while (popNode(&queue, &node) != NULL) {
			int row = node.row;
			int col = node.col;
			// Single Step QLearning
			QLearning(QTables, RTable, target, node);
			
			// add unregistered neighbors to queue and registry
			for (int i=0; i<4; i++)
				if ((RTable.map[row][col].ENWS[i] >= 0) && (!registry[row-quicksin(i)][col+quickcos(i)])) {
					if (addNode(&queue, row-quicksin(i), col+quickcos(i)))
						return -1;
					registry[row-quicksin(i)][col+quickcos(i)] = 1;
				}
		}
		
		// free memory allocated for registry
		for (int row=0; row < sSpace.height; row++)
			free(registry[row]);
		free(registry);
		// free memory allocated for RTable
		cleanupSSpace(RTable);
	}
	
	printQTables(QTables);
	
	return 0;
}

// builds a stateSpace where edges connecting to goal rooms are heavily incentivized
int buildRewardTable(struct stateSpace *RTable, struct stateSpace sSpace, char *goalRoom) {
	// store a copy of sSpace as RTable
	if (copySSpace(RTable, sSpace) < 0)
		return -1;
	
	// search RTable->map for states named by goalRoom
	// and incentivize all edges which lead to states named goalRoom
	for (int row=0; row < RTable->height; row++)
		for (int col=0; col < RTable->width; col++)
			if (RTable->map[row][col].name == goalRoom) // note: addresses are shared, shorthand comparison OK
				for (int i=0; i<4; i++)
					if (RTable->map[row][col].ENWS[i] >= 0) // since all connectinos are two-way
						RTable->map[row-quicksin(i)][col+quickcos(i)].ENWS[(i+2)%4] = 100; // arbitrarily big reward
	return 0;
}

// calculates the Q value for the given row and col
void QLearning(struct stateSpace QTables[], struct stateSpace RTable, struct pointNode target, struct pointNode position) {
	// calculate which QTable to use based on target and position
	int Q_i = pickQTable(position, target);
	
	// calculate Q value for 4 directions from position
	for (int d=0; d<4; d++) {
		// only Q-update valid directions
		if (QTables[Q_i].map[position.row][position.col].ENWS[d] >= 0) {
			struct pointNode nextPos;
			nextPos.row = position.row-quicksin(d); // row coords are upside down comp. to convention
			nextPos.col = position.col+quickcos(d);
			
			// retrieve maximum QValue option as if position had moved along d
			int nextQ_i = pickQTable(nextPos, target);
			int maxQ = RTable.map[position.row][position.col].ENWS[d]; // R value for action
			if (QTables[Q_i].map[position.row][position.col].ENWS[d] > maxQ)
				maxQ = QTables[Q_i].map[position.row][position.col].ENWS[d]; // current QValue for action
			for (int d2=0; d2<4; d2++)
				if (QTables[nextQ_i].map[nextPos.row][nextPos.col].ENWS[d2] > maxQ)
					maxQ = 0.8*QTables[nextQ_i].map[nextPos.row][nextPos.col].ENWS[d2]; // discounted QValue for delayed action
			
			// store QValue for this d
			QTables[Q_i].map[position.row][position.col].ENWS[d] = maxQ;
		}
	}
}


// provides a computationally cheap function to compute sin(0),sin(PI/2),sin(PI),sin(3PI/2)
int quicksin(int halfPis) {
	if (halfPis%4 == 1) return 1;
	if (halfPis%4 == 3) return -1;
	else return 0;
}
// provides a computationally cheap function to compute cos(0),cos(PI/2),cos(PI),cos(3PI/2)
int quickcos(int halfPis) {
	if (halfPis%4 == 0) return 1;
	if (halfPis%4 == 2) return -1;
	else return 0;
}


// picks a QTable based on direction and distance from target using polar coordinates
int pickQTable(struct pointNode pos, struct pointNode target) {
	// lengths between points
	// dy is flipped to match polar convention since row increases in downward direction
	double dx = target.col-pos.col;
	double dy = pos.row-target.row;
	double PI = 3.14159265359;
	
	// calculate the angle, CCW from eastern direction
	double angle = atan2(dy, dx);
	
	// calculate distance from pos to target
	double distance = sqrt((dx*dx)+(dy*dy));
	
	// use angle to determine which of 4 directions
	int direction;
	if (angle <= PI/4 && angle >= -PI/4)          // EAST
		direction = 0;
	else if (angle <= 3*PI/4 && angle >= PI/4)   // NORTH
		direction = 1;
	else if (angle >= -3*PI/4 && angle <= -PI/4) // SOUTH
		direction = 3;
	else                                        // WEST
		direction = 2;
	
	// use distance to determine which of 3 distances
	int dist = distance / 5;
	if (dist > 2)
		dist = 2;
	
	// return index of chosen QTable
	//fprintf(stderr, "t:%d,%d p:%d,%d - dy:%.3f dx:%.3f - a:%.4f d:%.1f - dir:%c dis:%d\n", target.row,target.col, pos.row, pos.col, dy, dx, angle, distance, (direction < 3 ? (direction < 2 ? (direction < 1 ? 'E' : 'N') : 'W') : 'S'), dist);
	
	return dist+direction*3;
}


void printQTables(struct stateSpace QTables[]) {
	fprintf(stdout, "QTable for targets NEAR & EAST\n");
	printQTable(QTables[0]);
 	fprintf(stdout, "\nQTable for targets MIDDISTANT & EAST\n");
 	printQTable(QTables[1]);
 	fprintf(stdout, "\nQTable for targets FAR & EAST\n");
 	printQTable(QTables[2]);
 	fprintf(stdout, "\nQTable for targets NEAR & NORTH\n");
 	printQTable(QTables[3]);
 	fprintf(stdout, "\nQTable for targets MIDDISTANT & NORTH\n");
 	printQTable(QTables[4]);
 	fprintf(stdout, "\nQTable for targets FAR & NORTH\n");
 	printQTable(QTables[5]);
 	fprintf(stdout, "\nQTable for targets NEAR & WEST\n");
 	printQTable(QTables[6]);
 	fprintf(stdout, "\nQTable for targets MIDDISTANT & WEST\n");
 	printQTable(QTables[7]);
 	fprintf(stdout, "\nQTable for targets FAR & WEST\n");
 	printQTable(QTables[8]);
 	fprintf(stdout, "\nQTable for targets NEAR & SOUTH\n");
 	printQTable(QTables[9]);
 	fprintf(stdout, "\nQTable for targets MIDDISTANT & SOUTH\n");
 	printQTable(QTables[10]);
 	fprintf(stdout, "\nQTable for targets FAR & SOUTH\n");
 	printQTable(QTables[11]);
}

void printQTable(struct stateSpace QTable) {
	for (int row=0; row<QTable.height; row++) {
		for (int col=0; col<QTable.width; col++) { // northern edge
			if (QTable.map[row][col].ENWS[1] >=0)
				fprintf(stdout, "  %3d     ", QTable.map[row][col].ENWS[1]);
			else
				fprintf(stdout, "          ");
		}
		fprintf(stdout, "\n");
		for (int col=0; col<QTable.width; col++) {
			if (QTable.map[row][col].ENWS[2] >=0) // west edge
				fprintf(stdout, "%3d", QTable.map[row][col].ENWS[2]);
			else
				fprintf(stdout, "   ");
			fprintf(stdout, " %c ", QTable.map[row][col].name ? 'X' : ' '); // table cell
			if (QTable.map[row][col].ENWS[0] >= 0)
				fprintf(stdout, "%-3d ", QTable.map[row][col].ENWS[0]); // east edge
			else
				fprintf(stdout, "    ");
		}
		fprintf(stdout, "\n");
		for (int col=0; col<QTable.width; col++) { // southern edge
			if (QTable.map[row][col].ENWS[3] >= 0)
				fprintf(stdout, "  %3d     ", QTable.map[row][col].ENWS[3]);
			else
				fprintf(stdout, "          ");
		}
		fprintf(stdout, "\n");
	}
}



