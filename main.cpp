#include <iostream>
#include "heist.h"
using namespace std;

// Test 1: files and rules (Module 2 and 6).
void testFilesAndRules() {
    cout << "===== TEST 1: Rules and About files =====" << endl;
    GameEngine& engine = GameEngine::getInstance();
    engine.loadRules();
    engine.showRules();
    cout << endl;
    engine.showAbout();
}

// Test 2: all seven culprit profiles load correctly from the text files (Module 6 and 12).
void testAllProfiles() {
    cout << endl << "===== TEST 2: All 7 culprit profiles =====" << endl;
    GameEngine& engine = GameEngine::getInstance();

    for (int k = 0; k < 7; k++) {
        engine.startNewGame("Test_Detective", k);

        // Collect every letter: 7 from the cottage and 8 from the people.
        for (int i = 0; i < engine.getCottage().getLocationCount(); i++) {
            engine.getBoard().placeLetter(engine.getCottage().searchLocation(i));
        }
        for (int i = 0; i < engine.getPeopleCount(); i++) {
            engine.getBoard().placeLetter(engine.getPerson(i)->getClue());
        }

        string w1 = engine.getBoard().getWord(WORD_1);
        string w2 = engine.getBoard().getWord(WORD_2);
        bool correct = engine.getBoard().checkWord(WORD_1, w1) && engine.getBoard().checkWord(WORD_2, w2);

        int culpritCount = 0;
        for (int i = 0; i < engine.getPeopleCount(); i++) {
            if (engine.getPerson(i)->getIsCulprit()) culpritCount++;
        }
        cout << "Culprit " << engine.getCulpritRole() << ": " << w1 << " + " << w2
            << " | words " << (correct ? "OK" : "WRONG")
            << " | culprits marked: " << culpritCount << endl;
    }
}

// Test 3: operators and friend functions (Module 11).
void testOperators() {
    cout << endl << "===== TEST 3: Operators =====" << endl;
    GameEngine& engine = GameEngine::getInstance();
    engine.startNewGame("Agent_Halim", 2);

    Tile a = { 'A', true };
    Tile b = { 'A', true };
    Tile c = { 'B', true };
    cout << "Tile " << a << " == Tile " << b << " : " << (a == b ? "true" : "false") << endl;
    cout << "Tile " << a << " == Tile " << c << " : " << (a == c ? "true" : "false") << endl;

    cout << *engine.getPerson(3) << endl;        // Character insertion operator
    Location loc = { "Kitchen", 'E', 5, WORD_1, false };
    cout << loc << endl;                         // Location insertion operator

    // Operator +: merge the two racks of the detective into one clue.
    Detective& d = engine.getDetective();
    d.getRack(WORD_1).addTile('L', 1);
    d.getRack(WORD_1).addTile('O', 2);
    d.getRack(WORD_2).addTile('H', 1);
    d.getRack(WORD_2).addTile('E', 2);
    TileRack merged = d.getRack(WORD_1) + d.getRack(WORD_2);
    cout << "Merged clue letters: [" << merged.getLetters() << "]" << endl;
    cout << "Merged rack: ";
    merged.display();

    engine.getBoard().placeLetter(engine.getCottage().searchLocation(0));
    cout << "Board:" << endl << engine.getBoard();  // Scrabble insertion operator
}

// Test 4: score board and file exceptions (Module 6 and 7).
void testScoreBoard() {
    cout << endl << "===== TEST 4: Score board =====" << endl;
    ScoreBoard real("leader/scoreBoard.txt");
    real.displayAll();

    // A separate test file is used, so the real score board stays clean.
    ScoreBoard temp("testScores.txt");
    temp.addScore("Test_Detective", "WIN", 70, 12, "Servant");
    cout << endl << "Test file after addScore:" << endl;
    temp.displayAll();

    cout << endl;
    try {
        ScoreBoard missing("nofile.txt");
        missing.displayAll();
    }
    catch (GameException& e) { cout << e.what() << endl; }
}

int main() {
    try {
        GameEngine& engine = GameEngine::getInstance();
        ScoreBoard scoreBoard("leader/scoreBoard.txt");
        engine.setScoreBoard(&scoreBoard);

        testFilesAndRules();
        testAllProfiles();
        testOperators();
        testScoreBoard();
    }
    catch (GameException& e) {
        cout << e.what() << endl;
    }
    GameEngine::destroyInstance();
    return 0;
}
