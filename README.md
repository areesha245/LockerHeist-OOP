THE LOCKER HEIST

A Detective Game backed by Principles of OOP

CL1004 - Object Oriented Programming | Semester Project | Fall 2026 National University of Computer & Emerging Sciences, Faisalabad-Chiniot Campus

Team Members
Name	Roll Number
Areesha Matloob	25F-3007
Breera Mushtaq	25F-3025
Section: BSE-3A
Lab Instructor: Mr. Mozeb Ahmed Khan
About the Game

The Locker Heist is a single-player, menu-driven detective mystery in C++. A locker hidden behind a bookshelf in the Study Room was robbed during a party. The detective must visit seven locations and interview eight people (the victim and seven suspects). Every location and every person gives one letter and its board position. Locations fill Word 1 (7 letters) and people fill Word 2 (8 letters) on a Scrabble-style board. When both words are formed, together they point to the culprit's role, who must then be accused before credibility or turns run out.

The culprit is chosen at random for every new game. Maximum turns, starting credibility and penalties are read from rules.txt.

Build and Run Instructions
Install Visual Studio with the "Desktop development with C++" workload.
Clone or download this repository and open the .sln file (or create an Empty C++ project and add the three source files).
Make sure heist.h is in Header Files, and heist.cpp and main.cpp are in Source Files.
Keep all text files in the same folder as main.cpp (the project folder), with the score board inside a leader folder:
clue.txt, locations.txt, characters.txt, rules.txt, about.txt
leader/scoreBoard.txt
Press Ctrl+F5 (Start Without Debugging) to build and run.

No STL containers or algorithms are used (only string, file streams and basic C library functions such as rand, time, atoi and system).

Project Files
File	Purpose
heist.h	Structs, enums, exception classes, DynamicArray<T> template and all class declarations
heist.cpp	Definitions of all classes and operators
main.cpp	Driver: creates the GameEngine, runs it and catches every GameException
clue.txt	Culprit role, Word 1 and Word 2 of the seven profiles
locations.txt	Location letters and positions for each profile
characters.txt	Names, roles, nationalities, positions and letters of the eight people
rules.txt	Max turns, starting credibility and penalties
about.txt	Team details and game description
leader/scoreBoard.txt	Saved results of finished games
Modules Implemented

Modules 1 to 14: 3-file structure, structs and enums, game initialization, character hierarchy (with virtual inheritance for PA), Cottage and Scrabble boards, TileRack, file handling with the score board, custom exceptions, DynamicArray<T> template, abstract Character with polymorphism, class relationships (composition, association, aggregation), operator overloading and friends, Singleton GameEngine, menus and completion conditions.

Deviations from the Manual
Repeating a visit or interview: if the player answers Yes to search or interview again, the turn is used and 5 credibility is lost (treated as a wrong step). Answering No costs nothing.
Refused interview: if credibility is below the person's required level, the person refuses, one turn is used and 5 credibility is lost.
Wrong accusation: costs one turn and 5 credibility.
Wrong word: costs one turn and 10 credibility. The detective may try again.
Invalid input (wrong menu number, incomplete row, wrong word length) costs nothing and shows a HEIST ERROR message.
Quit returns to the main menu and the game is not saved on the score board.
Score board stores Turns_Left (maximum turns minus turns used), as in the given file format.
characters.txt contains 8 data lines (the victim is included, because the victim is interviewed).
Roles are written with underscores in the text files (for example Partner_1) so that lines can be split safely on |.
The merged clue (TileRack + TileRack) shows letters in the order they were collected.
Known Limitations
The game is console-only and single-player, with no graphical interface.
A game cannot be saved and resumed; closing the program loses the current game.
Text files must be in the working folder of the program; otherwise a HEIST ERROR file exception is shown (rules fall back to defaults).
Screen clearing uses system("cls"), which is written for Windows (a clear fallback is included for other systems).
The detective name must be one word (spaces are replaced by _ or .).
