#ifndef HEIST_H
#define HEIST_H

#include <string>
using namespace std;

// ---------- ENUMERATIONS ----------

// Tracks which stage of the program the player is currently in.
// Named constants are easier to read than plain numbers.
enum GameState { MENU, PLAYING, WON, LOST, EXIT_GAME };

// Options of the main menu; values start from 1 to match the menu numbers.
enum MenuChoice { NEW_GAME = 1, LEADER_BOARD, ABOUT, RULES, EXIT_MENU };

// Tells whether a letter belongs to Word 1 (locations) or Word 2 (characters).
enum WordRow { WORD_1, WORD_2 };

// ---------- STRUCTURES ----------

// A single slot of the Scrabble board.
// A struct groups related data together, which improves code reusability.
struct Tile {
    char tileLetter;   // letter stored in this slot
    bool filled;       // true once the letter has been placed
};

// Carrier that moves a letter and its position from a
// Location or Character to the Scrabble board.
struct Clue {
    char clueLetter;
    int cluePosition;
    WordRow clueRow;
};

// One searchable place in the cottage (gives one letter of Word 1).
struct Location {
    string locName;
    char letterClueLoc;
    int boardPosLoc;
    WordRow locRow;
    bool locSearchFlag;   // true if already searched
};

// ---------- CHARACTER HIERARCHY ----------

// Abstract base class: holds data common to every person in the game.
// It cannot be instantiated because display() is pure virtual.
class Character {
protected:
    string characterName;
    string characterRole;
    string nationality;
    bool isCulprit;

public:
    Character(string name = "", string role = "", string nation = "");
    virtual ~Character();                       // virtual so derived objects are freed properly

    virtual void display() const = 0;           // every derived class must override this
    virtual int getRequiredCredibility() const; // minimum credibility needed to interview

    string getName() const;
    string getRole() const;
    string getNationality() const;
    bool getIsCulprit() const;
    void setIsCulprit(bool value);
};

// The victim (French business owner). Interviewed, but never a suspect.
class Victim : public Character {
private:
    WordRow clueRowChar;
    char letterClueChar;
    int boardPosChar;

public:
    Victim(string name, string nation, char letter, int pos);
    void display() const;
    int getRequiredCredibility() const;         // 80
    Clue getClue() const;
};

// The player. Stores credibility and turn count only (GameEngine increments turns).
class Detective : public Character {
private:
    int credibility;
    int turns;
    // TileRack will be added here in Module 5.

public:
    Detective(string name, string nation = "Malay", int startCredibility = 100);
    void display() const;
    void loseCredibility(int amount);
    void useTurn();
    int getCredibility() const;
    int getTurns() const;
};

// Base for all seven suspects. Stores the Word 2 letter data.
class Suspect : public Character {
protected:
    WordRow clueRowChar;
    char letterClueChar;
    int boardPosChar;

public:
    Suspect(string name, string role, string nation, char letter, int pos);
    Clue getClue() const;
    void setLetter(char letter);
};

// Virtual inheritance: PA reaches Suspect through two parents, so only one copy must exist.
class BusinessRelations : public virtual Suspect {
public:
    BusinessRelations(string name, string role, string nation, char letter, int pos);
};

class CloseRelations : public virtual Suspect {
public:
    CloseRelations(string name, string role, string nation, char letter, int pos);
};

class Partner : public BusinessRelations {
public:
    Partner(string name, string role, string nation, char letter, int pos);
    void display() const;
    int getRequiredCredibility() const;         // 50
};

// Diamond problem: PA inherits from both BusinessRelations and CloseRelations.
class PA : public BusinessRelations, public CloseRelations {
public:
    PA(string name, string nation, char letter, int pos);
    void display() const;
    int getRequiredCredibility() const;         // 35
};

class OwnersChild : public CloseRelations {
public:
    OwnersChild(string name, string nation, char letter, int pos);
    void display() const;
    int getRequiredCredibility() const;         // 75
};

// House help can be interviewed at any time (required credibility 0).
class HouseHelp : public Suspect {
public:
    HouseHelp(string name, string role, string nation, char letter, int pos);
};

class Cook : public HouseHelp {
public:
    Cook(string name, string nation, char letter, int pos);
    void display() const;
};

class Guard : public HouseHelp {
public:
    Guard(string name, string nation, char letter, int pos);
    void display() const;
};

class Servant : public HouseHelp {
public:
    Servant(string name, string nation, char letter, int pos);
    void display() const;
};

#endif
