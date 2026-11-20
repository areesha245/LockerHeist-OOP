#ifndef HEIST_H
#define HEIST_H

#include <iostream>
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

// ---------- OPERATORS FOR STRUCTS (Module 11) ----------

// Two tiles are equal when they hold the same letter (used to compare letters from clue.txt).
bool operator==(const Tile& a, const Tile& b);
bool operator!=(const Tile& a, const Tile& b);

// Insertion (<<) prints an object; extraction (>>) reads one from the keyboard.
ostream& operator<<(ostream& os, const Tile& t);
istream& operator>>(istream& is, Tile& t);
ostream& operator<<(ostream& os, const Location& loc);
istream& operator>>(istream& is, Location& loc);

// ---------- EXCEPTIONS (Module 7) ----------

// Base exception class for the whole game. Every message starts with HEIST ERROR.
// Derived classes describe one specific problem each, so main() can catch them by type.
class GameException {
protected:
    string message;

public:
    GameException(string msg) : message("HEIST ERROR: " + msg) {}
    virtual ~GameException() {}
    virtual string what() const { return message; }
};

// Thrown when a board position is outside the valid range of a word row.
class InvalidPositionException : public GameException {
public:
    InvalidPositionException() : GameException("Invalid board position.") {}
};

// Thrown when a letter is placed on a slot that already holds a letter.
class SlotFilledException : public GameException {
public:
    SlotFilledException() : GameException("This slot is already filled.") {}
};

// Thrown when a tile is added to a rack that has no free space.
class RackFullException : public GameException {
public:
    RackFullException() : GameException("The tile rack is full.") {}
};

// Thrown when a tile is taken from a rack that holds no tiles.
class RackEmptyException : public GameException {
public:
    RackEmptyException() : GameException("The tile rack is empty.") {}
};

// Thrown when a word is checked before its row is complete, or has a wrong length.
class InvalidWordException : public GameException {
public:
    InvalidWordException(string reason) : GameException(reason) {}
};

// Thrown by DynamicArray when an index is outside the stored elements.
class InvalidIndexException : public GameException {
public:
    InvalidIndexException() : GameException("Array index out of range.") {}
};

// Thrown when a text file is missing or its content is corrupted.
class FileException : public GameException {
public:
    FileException(string fileName) : GameException("File not found or corrupted: " + fileName) {}
};

// Thrown when the detective does not have enough credibility for an interview.
class LowCredibilityException : public GameException {
public:
    LowCredibilityException() : GameException("Not enough credibility for this interview.") {}
};

// Thrown when the player types something that is not a valid menu choice.
class InvalidInputException : public GameException {
public:
    InvalidInputException() : GameException("Invalid input. Please try again.") {}
};

// ---------- GENERIC DYNAMIC ARRAY (Module 8) ----------

// A generic (template) array that grows on its own.
// Templates utilize one piece of code for many types, which improves code reusability.
// Cottage, Scrabble and TileRack use it instead of raw new/delete arrays.
template <class T>
class DynamicArray {
private:
    T* data;        // heap memory that holds the elements
    int capacity;   // how many elements fit before growing
    int count;      // how many elements are in use

    // Doubles the capacity and copies the old elements into the new block.
    void grow() {
        int newCapacity = (capacity == 0) ? 4 : capacity * 2;
        T* bigger = new T[newCapacity]();
        for (int i = 0; i < count; i++) bigger[i] = data[i];
        delete[] data;
        data = bigger;
        capacity = newCapacity;
    }

public:
    // Allocates memory for the given number of elements; the array starts empty.
    DynamicArray(int initialCapacity = 4) : data(0), capacity(initialCapacity), count(0) {
        if (capacity > 0) data = new T[capacity]();
    }

    // Copy constructor: makes a deep copy so two arrays never share memory.
    DynamicArray(const DynamicArray<T>& other) : data(0), capacity(other.capacity), count(other.count) {
        if (capacity > 0) {
            data = new T[capacity]();
            for (int i = 0; i < count; i++) data[i] = other.data[i];
        }
    }

    // Assignment operator: frees old memory, then deep copies (self-assignment safe).
    DynamicArray<T>& operator=(const DynamicArray<T>& other) {
        if (this != &other) {
            delete[] data;
            capacity = other.capacity;
            count = other.count;
            data = 0;
            if (capacity > 0) {
                data = new T[capacity]();
                for (int i = 0; i < count; i++) data[i] = other.data[i];
            }
        }
        return *this;
    }

    // Destructor frees the heap memory so there is no leak.
    ~DynamicArray() { delete[] data; }

    // Adds an element at the end and grows the array if it is full.
    void add(const T& item) {
        if (count == capacity) grow();
        data[count] = item;
        count++;
    }

    // Sets the number of used elements to n; new elements get default values.
    void resizeTo(int n) {
        while (capacity < n) grow();
        for (int i = count; i < n; i++) data[i] = T();
        count = n;
    }

    // Removes the element at index and shifts later elements one step left.
    void removeAt(int index) {
        if (index < 0 || index >= count) throw InvalidIndexException();
        for (int i = index; i < count - 1; i++) data[i] = data[i + 1];
        count--;
    }

    // Bounds-checked access that allows reading and writing.
    T& operator[](int index) {
        if (index < 0 || index >= count) throw InvalidIndexException();
        return data[index];
    }

    // Bounds-checked access for const objects (read only).
    const T& operator[](int index) const {
        if (index < 0 || index >= count) throw InvalidIndexException();
        return data[index];
    }

    int getSize() const { return count; }
    void clear() { count = 0; }
};

// ---------- TILE RACK (Module 5) ----------

// Holds the letters (and their board positions) that the detective has collected.
// Responsibility: store collected tiles safely. Uses DynamicArray<Tile> internally.
class TileRack {
private:
    DynamicArray<Tile> tiles;      // collected letters
    DynamicArray<int> positions;   // board position of each collected letter
    int maxSize;                   // rack is full when this many tiles are stored

public:
    TileRack(int maxTiles = 8);
    TileRack(const TileRack& other);              // deep copy
    TileRack& operator=(const TileRack& other);   // deep copy assignment
    ~TileRack();

    void addTile(char letter, int position);
    void removeTile(int index);
    int getCount() const;
    bool isEmpty() const;
    bool isFull() const;
    void display() const;
    string getLetters() const;                    // all letters as one string

    // Merges two racks into one clue, with a space tile between them (Module 11).
    TileRack operator+(const TileRack& other) const;
};

// ---------- SCRABBLE BOARD (Module 4) ----------

// The two-row letter board. Row 1 has 7 slots (Word 1), Row 2 has 8 slots (Word 2).
// Responsibility: place letters at their positions and check the finished words.
class Scrabble {
private:
    DynamicArray<Tile> rowOne;    // 7 slots for Word 1
    DynamicArray<Tile> rowTwo;    // 8 slots for Word 2
    string targetWord1;           // correct Word 1 of the chosen culprit
    string targetWord2;           // correct Word 2 of the chosen culprit

    int rowLength(WordRow row) const;

public:
    Scrabble();
    Scrabble(const Scrabble& other);              // deep copy
    Scrabble& operator=(const Scrabble& other);   // deep copy assignment
    ~Scrabble();

    void setTargetWords(string word1, string word2);
    void placeLetter(const Clue& clue);
    bool isRowComplete(WordRow row) const;
    string getWord(WordRow row) const;
    bool checkWord(WordRow row, string guess) const;
    void displayBoard() const;

    // Friend functions can reach the private rows of the board (Module 11).
    friend ostream& operator<<(ostream& os, const Scrabble& board);
    friend istream& operator>>(istream& is, Scrabble& board);
};

// ---------- COTTAGE (Module 4) ----------

// The cottage has seven locations. Responsibility: store and search locations.
// Aggregation: the Cottage holds a DynamicArray of Location structs (LocCache).
class Cottage {
private:
    DynamicArray<Location> LocCache;

public:
    Cottage();
    Cottage(const Cottage& other);                // deep copy
    Cottage& operator=(const Cottage& other);     // deep copy assignment
    ~Cottage();

    void addLocation(const Location& loc);
    int getLocationCount() const;
    bool isSearched(int index) const;
    string getLocationName(int index) const;
    Clue searchLocation(int index);
    void displayLocations() const;
};

// ---------- LEADER / SCORE BOARD (Module 6) ----------

// Saves finished games to a text file and shows all earlier games.
// Responsibility: read and write the score board file.
class ScoreBoard {
private:
    string fileName;

public:
    ScoreBoard(string file = "leader/scoreBoard.txt");
    void addScore(string detective, string result, int credibility, int turnsLeft, string culprit);
    void displayAll() const;
};

// ---------- CHARACTER HIERARCHY ----------

class GameEngine;   // forward declaration, needed for the friend declaration below

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
    virtual Clue getClue() const;               // letter and position this person gives

    string getName() const;
    string getRole() const;
    string getNationality() const;
    bool getIsCulprit() const;
    void setIsCulprit(bool value);

    // GameEngine is a friend class: it may read and change the private/protected data directly.
    friend class GameEngine;
    friend ostream& operator<<(ostream& os, const Character& c);
    friend istream& operator>>(istream& is, Character& c);
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

// The player. Stores credibility, turn count and the two racks (GameEngine increments turns).
class Detective : public Character {
private:
    int credibility;
    int turns;
    TileRack rackWord1;   // letters collected for Word 1 (locations)
    TileRack rackWord2;   // letters collected for Word 2 (characters)

public:
    Detective(string name, string nation = "Malay", int startCredibility = 100);
    void display() const;
    void loseCredibility(int amount);
    void useTurn();
    int getCredibility() const;
    int getTurns() const;
    TileRack& getRack(WordRow row);
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

// ---------- GAME ENGINE (Module 12) ----------

// Singleton: only one GameEngine object can exist, and it runs the whole game.
// Composition: it owns the Detective, Cottage, Scrabble board, clue rack and the characters.
// Association: it only uses the ScoreBoard (it does not own it).
class GameEngine {
private:
    static GameEngine* instance;     // the one and only object

    // Rules (defaults are used when rules.txt is missing or corrupt)
    int maxTurns;
    int startCredibility;
    int wrongTurnPenalty;
    int wrongWordPenalty;
    string winCondition;
    string loseCondition;
    bool rulesFromFile;

    int culpritIndex;                // 0 to 6, position of the culprit profile
    string culpritRole;

    Detective* detective;            // composition (created for every new game)
    Cottage cottage;                 // composition
    Scrabble board;                  // composition
    TileRack clueRack;               // composition (merged rack used as the final clue)
    Character* people[8];            // victim + seven suspects
    int peopleCount;
    ScoreBoard* scoreBoard;          // association
    GameState state;

    // Constructor, copy constructor and assignment are private, so no second object can be made.
    GameEngine();
    GameEngine(const GameEngine&);
    GameEngine& operator=(const GameEngine&);
    ~GameEngine();

    void clearGame();
    void loadClues(string roles[], string words1[], string words2[]);
    void loadLocations(int profile);
    void loadCharacters(int profile);

public:
    static GameEngine& getInstance();
    static void destroyInstance();

    void setScoreBoard(ScoreBoard* board);
    void loadRules();
    void showRules() const;
    void showAbout() const;
    void startNewGame(string detectiveName, int forcedCulprit = -1);

    Detective& getDetective();
    Cottage& getCottage();
    Scrabble& getBoard();
    TileRack& getClueRack();
    Character* getPerson(int index);
    int getPeopleCount() const;
    string getCulpritRole() const;
    int getMaxTurns() const;
    int getWrongTurnPenalty() const;
    int getWrongWordPenalty() const;
    GameState getState() const;
    void setState(GameState newState);
};

#endif

