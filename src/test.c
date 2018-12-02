/* ***********************************************************************
 * Program: test.c
 * Description: Framework for building, training, and testing a robot
 *      using Q-Learning Algorithm to navigate map represented by a csv file
 * Author: Samuel Shinn
 * Last Modified: 12/14/2017
 * 
 * NOTES:
 *  In this file there is only the main function since almost all data 
 *   in main is relevant throughout the program's entire execution
 *   and each step is uniquely executed once.
 *  In reading the code for this program, I recommend viewing files as
 *   their functions are called. i.e.
 *    parseArgs.c
 *    stateSpace.c
 *    QTable.c
 * ***********************************************************************
 */


#include "parseArgs.c"
#include "QTable.c"


int main(int argc, char** argv) {
	// get arguments
	struct paramaters params;
	if (parseArgs(argc, argv, &params) < 0)
		return 1; // error, quit program
	printParams(params);
	
	// create stateSpace from input file GridMap
	struct stateSpace sSpace;
	if (buildStateSpace(params.filename, &sSpace) < 0)
		return 1;
	
	//printMap(sSpace);
	
	// create QTables from stateSpace
	int directions = 4; // should be managed by params
	int distances = 3; // should be managed by params
	struct stateSpace QTables[directions*distances];
	if (buildQTables(QTables, sSpace, directions*distances) < 0)
		return 1;
	
	// train QTables
	train(sSpace, QTables);
	
	// test QTables
	
	
	return 0;
}




