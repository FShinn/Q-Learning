# Q-Learning

An implementation of a Q-Learning algoirthm used to navigate some map provided as input data.
View [readme.txt](readme.txt) for usage details.

## About

The goal of this project is to design a program which will allow a virtual robot to traverse a
virtual representation of the 4th floor of WWU’s Communications Facility (CF) building using a
Q-Learning algorithm. By using the Q-Learning algorithm, there should be no need to program
the various routes into the program, nor should the robot be required to recompute any route
when requested to travel. Instead, after training the robot, the robot will have QTable (or multiple
QTables) which it may reference ‘at-a-glance’ in order to gather instructions to reach the goal
destination. For this particular program, our objective is to allow for more than one goal
destination. The user will be able to, at runtime, order the robot to go to any room on the 4th
floor of the CF building. Then, as another addition, the robot will return to an area indicated as
‘home’.


For a complete report, view [Report.pdf](doc/Report.pdf) in the doc folder.

## Author

* **Sam (Forrie) Shinn** - *Sole Contributor* - [FShinn](https://github.com/FShinn)

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details
