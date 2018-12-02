Sam Shinn
Q-Learning

Files
 executable:
  test.exe
 source:
  test.c
  parseArgs.c
  stateSpace.c
  QTable.c
 data:
  GridMap.csv
 misc:
  readme.txt (this file)
  outputFiles (directory containing output of test runs)
  
  
  
COMPILING TEST.C
test.c can compile on Visual C++ 2015 x86 Native Build Tools Command Prompt using the command prompt:

  gcc -o test test.c

compiling has not been tested on any other environments. 



RUNNING TEST.EXE
The test.exe file included in SamuelShinnQLearning.zip was compiled using Visual C++ 2015 x86 Native
Build Tools Command Prompt, and will run in any Windows command prompt. 
The basic command to run the program is

  ./test filename homename
  
 where "filename" is the name of a csv file containing room names, e.g. "GridMap.csv" and "homename" is 
 the name of a room present in "filename", e.g. "411".
The program assumes that the first line of the csv file will be a comma seperated list of column names,
 and will skip it for the purposes of reading inputs.
