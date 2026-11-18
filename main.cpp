#include <iostream>
#include "heist.h"
using namespace std;

// Test 1: polymorphism over the Character hierarchy (Module 3 and 9).
void testCharacters() {
    cout << "===== TEST 1: Characters =====" << endl;
    Character* people[8];
    people[0] = new Victim("Mr.Leonardo", "French", 'S', 1);
    people[1] = new Detective("Agent_Halim");
    people[2] = new Cook("Mr.Enzo", "Italian", 'P', 2);
    people[3] = new Guard("Mr.Lukas", "German", 'A', 3);
    people[4] = new Servant("Mr.Toby", "British", 'G', 4);
    people[5] = new OwnersChild("Ms.Celine", "French", 'H', 5);
    people[6] = new Partner("Ms.Ruslana", "Partner_1", "Ukrainian", 'E', 6);
    people[7] = new PA("Mr.Jad", "Egyptian", 'I', 8);

    for (int i = 0; i < 8; i++) {
        people[i]->display();
        cout << "  Required credibility: " << people[i]->getRequiredCredibility() << endl;
    }
    for (int i = 0; i < 8; i++) delete people[i];
}

// Test 2: Cottage, Scrabble and TileRack working together (Module 4, 5, 8).
void testBoardAndRack() {
    cout << endl << "===== TEST 2: Cottage, Scrabble, TileRack =====" << endl;

    // Servant profile: Word 1 = LOOTERS
    Cottage cottage;
    string names[7] = { "Bedroom_1", "Bedroom_2", "Study_Room", "Garden", "Kitchen", "Dining_Room", "Living_Room" };
    char letters[7] = { 'L', 'O', 'O', 'T', 'E', 'R', 'S' };
    for (int i = 0; i < 7; i++) {
        Location loc = { names[i], letters[i], i + 1, WORD_1, false };
        cottage.addLocation(loc);
    }
    cottage.displayLocations();

    Scrabble board;
    board.setTargetWords("LOOTERS", "HERITAGE");
    Detective detective("Agent_Halim");

    // Search three locations; each letter goes to the rack and the board.
    int visit[3] = { 2, 0, 5 };
    for (int i = 0; i < 3; i++) {
        Clue c = cottage.searchLocation(visit[i]);
        detective.getRack(WORD_1).addTile(c.clueLetter, c.cluePosition);
        board.placeLetter(c);
    }
    cout << endl << "After 3 searches:" << endl;
    cottage.displayLocations();
    cout << "Rack 1: ";
    detective.getRack(WORD_1).display();
    board.displayBoard();

    // Complete Word 1 and check guesses.
    for (int i = 0; i < 7; i++) {
        if (!cottage.isSearched(i)) {
            Clue c = cottage.searchLocation(i);
            board.placeLetter(c);
        }
    }
    cout << endl << "After all 7 searches:" << endl;
    board.displayBoard();
    cout << "Guess LOOTERS: " << (board.checkWord(WORD_1, "looters") ? "CORRECT" : "WRONG") << endl;
    cout << "Guess RENOIRS: " << (board.checkWord(WORD_1, "RENOIRS") ? "CORRECT" : "WRONG") << endl;

    // Deep copy test: the copy must stay unchanged when the original changes.
    Cottage copyCottage(cottage);
    cottage.addLocation(Location());
    cout << "Original locations: " << cottage.getLocationCount()
        << " | Copy locations: " << copyCottage.getLocationCount() << endl;
}

// Test 3: every custom exception is thrown and caught (Module 7).
void testExceptions() {
    cout << endl << "===== TEST 3: Exceptions =====" << endl;
    Scrabble board;
    Clue good = { 'A', 3, WORD_1 };
    board.placeLetter(good);

    try { board.placeLetter(good); }
    catch (GameException& e) { cout << e.what() << endl; }

    try { Clue bad = { 'B', 9, WORD_1 }; board.placeLetter(bad); }
    catch (GameException& e) { cout << e.what() << endl; }

    try { board.checkWord(WORD_1, "LOOTERS"); }
    catch (GameException& e) { cout << e.what() << endl; }

    TileRack rack(2);
    try {
        rack.addTile('A', 1);
        rack.addTile('B', 2);
        rack.addTile('C', 3);
    }
    catch (GameException& e) { cout << e.what() << endl; }

    TileRack emptyRack(2);
    try { emptyRack.removeTile(0); }
    catch (GameException& e) { cout << e.what() << endl; }
}

int main() {
    try {
        testCharacters();
        testBoardAndRack();
        testExceptions();
    }
    catch (GameException& e) {
        cout << e.what() << endl;
    }
    return 0;
}