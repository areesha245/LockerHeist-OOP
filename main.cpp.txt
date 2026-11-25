#include <iostream>
#include "heist.h"
using namespace std;

// Starts the game. Every GameException is caught here; after an error the engine resumes.
int main() {
    ScoreBoard scoreBoard("leader/scoreBoard.txt");
    GameEngine& engine = GameEngine::getInstance();
    engine.setScoreBoard(&scoreBoard);
    engine.loadRules();

    bool finished = false;
    while (!finished) {
        try {
            engine.run();
            finished = true;
        }
        catch (GameException& e) {
            cout << endl << e.what() << endl;   // message starts with HEIST ERROR
        }
    }

    GameEngine::destroyInstance();
    return 0;
}
