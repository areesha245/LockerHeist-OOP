#include <iostream>
#include "heist.h"
using namespace std;

// ---------- Helper ----------
// Converts a word to capital letters so that "pyramids" equals "PYRAMIDS".
static string toUpperWord(string word) {
    for (int i = 0; i < (int)word.length(); i++) {
        if (word[i] >= 'a' && word[i] <= 'z') word[i] = word[i] - 32;
    }
    return word;
}

// ---------- TileRack ----------
// Earlier raw DMA version (kept as a comment, as the manual requires):
//   Tile* tileArray;  int tileCount;
//   tileArray = new Tile[maxSize];  ...  delete[] tileArray;
// DynamicArray<Tile> now does the same job safely.

// Creates an empty rack that can hold maxTiles letters.
TileRack::TileRack(int maxTiles) : tiles(maxTiles), positions(maxTiles), maxSize(maxTiles) {}

// Deep copy: the DynamicArray members copy their own memory.
TileRack::TileRack(const TileRack& other)
    : tiles(other.tiles), positions(other.positions), maxSize(other.maxSize) {
}

// Assignment copies every member, so both racks stay independent.
TileRack& TileRack::operator=(const TileRack& other) {
    if (this != &other) {
        tiles = other.tiles;
        positions = other.positions;
        maxSize = other.maxSize;
    }
    return *this;
}

// Memory is freed automatically by the DynamicArray destructors.
TileRack::~TileRack() {}

// Stores a letter and its board position; throws if the rack is full.
void TileRack::addTile(char letter, int position) {
    if (isFull()) throw RackFullException();
    Tile t = { letter, true };
    tiles.add(t);
    positions.add(position);
}

// Removes the tile at index; throws if the rack is empty.
void TileRack::removeTile(int index) {
    if (isEmpty()) throw RackEmptyException();
    tiles.removeAt(index);
    positions.removeAt(index);
}

int TileRack::getCount() const { return tiles.getSize(); }
bool TileRack::isEmpty() const { return tiles.getSize() == 0; }
bool TileRack::isFull() const { return tiles.getSize() >= maxSize; }

// Shows every stored letter with its board position, e.g. A(3) P(1).
void TileRack::display() const {
    if (isEmpty()) {
        cout << "(empty)" << endl;
        return;
    }
    for (int i = 0; i < tiles.getSize(); i++) {
        cout << tiles[i].tileLetter << "(" << positions[i] << ") ";
    }
    cout << endl;
}

// ---------- Scrabble ----------
// Earlier raw DMA version (kept as a comment):
//   Tile** grid = new Tile*[2];  grid[0] = new Tile[7];  grid[1] = new Tile[8];
//   ...  delete[] grid[0];  delete[] grid[1];  delete[] grid;

// Creates both rows with every slot empty.
Scrabble::Scrabble() : rowOne(7), rowTwo(8), targetWord1(""), targetWord2("") {
    rowOne.resizeTo(7);
    rowTwo.resizeTo(8);
}

// Deep copy of both rows and the target words.
Scrabble::Scrabble(const Scrabble& other)
    : rowOne(other.rowOne), rowTwo(other.rowTwo),
    targetWord1(other.targetWord1), targetWord2(other.targetWord2) {
}

// Assignment operator performs a deep copy as well.
Scrabble& Scrabble::operator=(const Scrabble& other) {
    if (this != &other) {
        rowOne = other.rowOne;
        rowTwo = other.rowTwo;
        targetWord1 = other.targetWord1;
        targetWord2 = other.targetWord2;
    }
    return *this;
}

Scrabble::~Scrabble() {}

// Returns 7 for Word 1 and 8 for Word 2.
int Scrabble::rowLength(WordRow row) const {
    return (row == WORD_1) ? 7 : 8;
}

// Stores the correct words (loaded from clue.txt) of the chosen culprit.
void Scrabble::setTargetWords(string word1, string word2) {
    targetWord1 = toUpperWord(word1);
    targetWord2 = toUpperWord(word2);
}

// Places a letter at its position; throws if the position is wrong or already filled.
void Scrabble::placeLetter(const Clue& clue) {
    if (clue.cluePosition < 1 || clue.cluePosition > rowLength(clue.clueRow))
        throw InvalidPositionException();

    Tile& slot = (clue.clueRow == WORD_1) ? rowOne[clue.cluePosition - 1]
        : rowTwo[clue.cluePosition - 1];
    if (slot.filled) throw SlotFilledException();

    slot.tileLetter = clue.clueLetter;
    slot.filled = true;
}

// A row is complete when every one of its slots holds a letter.
bool Scrabble::isRowComplete(WordRow row) const {
    const DynamicArray<Tile>& r = (row == WORD_1) ? rowOne : rowTwo;
    for (int i = 0; i < r.getSize(); i++) {
        if (!r[i].filled) return false;
    }
    return true;
}

// Builds the string of letters on the board; empty slots appear as '_'.
string Scrabble::getWord(WordRow row) const {
    const DynamicArray<Tile>& r = (row == WORD_1) ? rowOne : rowTwo;
    string word = "";
    for (int i = 0; i < r.getSize(); i++) {
        word += r[i].filled ? r[i].tileLetter : '_';
    }
    return word;
}

// Compares the detective's guess with the correct word; the row must be complete.
bool Scrabble::checkWord(WordRow row, string guess) const {
    if (!isRowComplete(row)) throw InvalidWordException("This row is not complete yet.");
    guess = toUpperWord(guess);
    if ((int)guess.length() != rowLength(row)) throw InvalidWordException("Word has the wrong length.");
    string target = (row == WORD_1) ? targetWord1 : targetWord2;
    return guess == target;
}

// Prints both rows: [ ] for an empty slot, [X] for a filled one.
void Scrabble::displayBoard() const {
    for (int w = 0; w < 2; w++) {
        WordRow row = (w == 0) ? WORD_1 : WORD_2;
        const DynamicArray<Tile>& r = (w == 0) ? rowOne : rowTwo;
        cout << "  Word " << (w + 1) << ": ";
        for (int i = 0; i < r.getSize(); i++) {
            cout << "[" << (r[i].filled ? r[i].tileLetter : ' ') << "] ";
        }
        if (isRowComplete(row)) cout << " COMPLETE";
        cout << endl;
    }
}

// ---------- Cottage ----------
// Earlier raw DMA version (kept as a comment):
//   Location* LocCache;  LocCache = new Location[7];  ...  delete[] LocCache;

// Reserves room for the seven locations of the cottage.
Cottage::Cottage() : LocCache(7) {}

// Deep copy of all locations.
Cottage::Cottage(const Cottage& other) : LocCache(other.LocCache) {}

// Assignment operator performs a deep copy as well.
Cottage& Cottage::operator=(const Cottage& other) {
    if (this != &other) LocCache = other.LocCache;
    return *this;
}

// The DynamicArray destructor deletes the location array itself.
Cottage::~Cottage() {}

// Adds one location (name, letter, position) to the cottage.
void Cottage::addLocation(const Location& loc) { LocCache.add(loc); }

int Cottage::getLocationCount() const { return LocCache.getSize(); }

// Tells whether the location at index was searched earlier.
bool Cottage::isSearched(int index) const { return LocCache[index].locSearchFlag; }

string Cottage::getLocationName(int index) const { return LocCache[index].locName; }

// Marks the location as searched and returns its letter as a Clue.
Clue Cottage::searchLocation(int index) {
    Location& loc = LocCache[index];
    loc.locSearchFlag = true;
    Clue c = { loc.letterClueLoc, loc.boardPosLoc, loc.locRow };
    return c;
}

// Prints all locations with their search status.
void Cottage::displayLocations() const {
    for (int i = 0; i < LocCache.getSize(); i++) {
        cout << "[" << (i + 1) << "] " << LocCache[i].locName << " - "
            << (LocCache[i].locSearchFlag ? "Searched" : "NOT Searched") << endl;
    }
}

// ---------- Character ----------
// Constructor sets the common data; a person is not the culprit by default.
Character::Character(string name, string role, string nation)
    : characterName(name), characterRole(role), nationality(nation), isCulprit(false) {
}

// Virtual destructor makes sure derived destructors run when deleting via Character*.
Character::~Character() {}

// Default rule: anyone can be interviewed unless a derived class says otherwise.
int Character::getRequiredCredibility() const { return 0; }

string Character::getName() const { return characterName; }
string Character::getRole() const { return characterRole; }
string Character::getNationality() const { return nationality; }
bool Character::getIsCulprit() const { return isCulprit; }
void Character::setIsCulprit(bool value) { isCulprit = value; }

// ---------- Victim ----------
Victim::Victim(string name, string nation, char letter, int pos)
    : Character(name, "Victim", nation), clueRowChar(WORD_2), letterClueChar(letter), boardPosChar(pos) {
}

void Victim::display() const {
    cout << characterName << " (Victim, " << nationality << ")" << endl;
}

int Victim::getRequiredCredibility() const { return 80; }

// Packs the letter data into a Clue so it can be sent to the Scrabble board.
Clue Victim::getClue() const {
    Clue c = { letterClueChar, boardPosChar, clueRowChar };
    return c;
}

// ---------- Detective ----------
// The two racks hold 7 letters (Word 1) and 8 letters (Word 2).
Detective::Detective(string name, string nation, int startCredibility)
    : Character(name, "Detective", nation), credibility(startCredibility), turns(0),
    rackWord1(7), rackWord2(8) {
}

void Detective::display() const {
    cout << characterName << " (Detective) | Credibility: " << credibility
        << " | Turns used: " << turns << endl;
}

// Credibility never goes below zero.
void Detective::loseCredibility(int amount) {
    credibility -= amount;
    if (credibility < 0) credibility = 0;
}

void Detective::useTurn() { turns++; }
int Detective::getCredibility() const { return credibility; }
int Detective::getTurns() const { return turns; }

// Gives access to the rack of the requested word.
TileRack& Detective::getRack(WordRow row) {
    return (row == WORD_1) ? rackWord1 : rackWord2;
}

// ---------- Suspect and middle classes ----------
Suspect::Suspect(string name, string role, string nation, char letter, int pos)
    : Character(name, role, nation), clueRowChar(WORD_2), letterClueChar(letter), boardPosChar(pos) {
}

Clue Suspect::getClue() const {
    Clue c = { letterClueChar, boardPosChar, clueRowChar };
    return c;
}

// Used later when the culprit is selected and letters are loaded from the files.
void Suspect::setLetter(char letter) { letterClueChar = letter; }

// The Suspect initializer here is ignored when a more derived class builds the object.
BusinessRelations::BusinessRelations(string name, string role, string nation, char letter, int pos)
    : Suspect(name, role, nation, letter, pos) {
}

CloseRelations::CloseRelations(string name, string role, string nation, char letter, int pos)
    : Suspect(name, role, nation, letter, pos) {
}

HouseHelp::HouseHelp(string name, string role, string nation, char letter, int pos)
    : Suspect(name, role, nation, letter, pos) {
}

// ---------- Partner ----------
// The most derived class must initialize the virtual base Suspect itself.
Partner::Partner(string name, string role, string nation, char letter, int pos)
    : Suspect(name, role, nation, letter, pos), BusinessRelations(name, role, nation, letter, pos) {
}

void Partner::display() const {
    cout << characterName << " (" << characterRole << ", " << nationality << ")" << endl;
}
int Partner::getRequiredCredibility() const { return 50; }

// ---------- PA ----------
PA::PA(string name, string nation, char letter, int pos)
    : Suspect(name, "PA", nation, letter, pos),
    BusinessRelations(name, "PA", nation, letter, pos),
    CloseRelations(name, "PA", nation, letter, pos) {
}

void PA::display() const {
    cout << characterName << " (PA, " << nationality << ")" << endl;
}
int PA::getRequiredCredibility() const { return 35; }

// ---------- OwnersChild ----------
OwnersChild::OwnersChild(string name, string nation, char letter, int pos)
    : Suspect(name, "Child", nation, letter, pos), CloseRelations(name, "Child", nation, letter, pos) {
}

void OwnersChild::display() const {
    cout << characterName << " (Owner's Child, " << nationality << ")" << endl;
}
int OwnersChild::getRequiredCredibility() const { return 75; }

// ---------- House help ----------
Cook::Cook(string name, string nation, char letter, int pos)
    : HouseHelp(name, "Cook", nation, letter, pos) {
}
void Cook::display() const {
    cout << characterName << " (Cook, " << nationality << ")" << endl;
}

Guard::Guard(string name, string nation, char letter, int pos)
    : HouseHelp(name, "Guard", nation, letter, pos) {
}
void Guard::display() const {
    cout << characterName << " (Guard, " << nationality << ")" << endl;
}

Servant::Servant(string name, string nation, char letter, int pos)
    : HouseHelp(name, "Servant", nation, letter, pos) {
}
void Servant::display() const {
    cout << characterName << " (Servant, " << nationality << ")" << endl;
}