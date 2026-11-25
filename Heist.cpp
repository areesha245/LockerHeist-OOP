#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS   // lets Visual Studio accept localtime()
#endif

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
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

// Removes spaces, tabs and line-end characters from both ends of a text.
static string trimText(string text) {
    int start = 0;
    int end = (int)text.length() - 1;
    while (start <= end && (text[start] == ' ' || text[start] == '\t' || text[start] == '\r' || text[start] == '\n')) start++;
    while (end >= start && (text[end] == ' ' || text[end] == '\t' || text[end] == '\r' || text[end] == '\n')) end--;
    return text.substr(start, end - start + 1);
}

// Splits one line at every '|' into parts; returns how many parts the line really has.
static int splitLine(const string& line, string parts[], int maxParts) {
    int count = 0;
    string current = "";
    for (int i = 0; i < (int)line.length(); i++) {
        if (line[i] == '|') {
            if (count < maxParts) parts[count] = trimText(current);
            count++;
            current = "";
        }
        else {
            current += line[i];
        }
    }
    if (count < maxParts) parts[count] = trimText(current);
    count++;
    return count;
}

// Opens a data file, skips its header line and reads up to maxLines non-empty lines.
// Throws FileException when the file is missing or empty.
static int readDataLines(const string& fileName, string lines[], int maxLines) {
    ifstream file(fileName.c_str());
    if (!file.is_open()) throw FileException(fileName);
    string line;
    if (!getline(file, line)) throw FileException(fileName);
    int count = 0;
    while (count < maxLines && getline(file, line)) {
        if (trimText(line) == "") continue;
        lines[count] = line;
        count++;
    }
    return count;
}

// Converts a number from 0 to 99 into two digits, e.g. 5 becomes "05".
static string twoDigits(int n) {
    string s = "";
    s += char('0' + n / 10);
    s += char('0' + n % 10);
    return s;
}

// Returns today's date as dd-mm-yy, the same format used in leader/scoreBoard.txt.
static string todayDate() {
    time_t now = time(0);
    tm* t = localtime(&now);
    return twoDigits(t->tm_mday) + "-" + twoDigits(t->tm_mon + 1) + "-" + twoDigits((t->tm_year + 1900) % 100);
}

// ---------- Operators for structs ----------

// Tiles are equal when their letters are equal.
bool operator==(const Tile& a, const Tile& b) { return a.tileLetter == b.tileLetter; }
bool operator!=(const Tile& a, const Tile& b) { return !(a == b); }

// Prints a tile as [X], or [ ] when it is empty.
ostream& operator<<(ostream& os, const Tile& t) {
    os << "[" << (t.filled ? t.tileLetter : ' ') << "]";
    return os;
}

// Reads one letter from the keyboard into a tile.
istream& operator>>(istream& is, Tile& t) {
    is >> t.tileLetter;
    t.filled = true;
    return is;
}

// Prints a location with its search status.
ostream& operator<<(ostream& os, const Location& loc) {
    os << loc.locName << " (Word " << (loc.locRow == WORD_1 ? 1 : 2) << ", position "
        << loc.boardPosLoc << ", letter " << loc.letterClueLoc << ") - "
        << (loc.locSearchFlag ? "Searched" : "NOT Searched");
    return os;
}

// Reads name, letter and position of a location from the keyboard.
istream& operator>>(istream& is, Location& loc) {
    is >> loc.locName >> loc.letterClueLoc >> loc.boardPosLoc;
    loc.locRow = WORD_1;
    loc.locSearchFlag = false;
    return is;
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
        if (tiles[i].tileLetter == ' ') cout << "[space] ";
        else cout << tiles[i].tileLetter << "(" << positions[i] << ") ";
    }
    cout << endl;
}

// Returns all stored letters as one string, e.g. "LOT".
string TileRack::getLetters() const {
    string letters = "";
    for (int i = 0; i < tiles.getSize(); i++) letters += tiles[i].tileLetter;
    return letters;
}

// Operator +: joins two racks into one new rack with a space tile in the middle.
// The result is used as the single final clue to catch the culprit.
TileRack TileRack::operator+(const TileRack& other) const {
    TileRack merged(maxSize + other.maxSize + 1);
    for (int i = 0; i < tiles.getSize(); i++) merged.addTile(tiles[i].tileLetter, positions[i]);
    merged.addTile(' ', 0);
    for (int j = 0; j < other.tiles.getSize(); j++) merged.addTile(other.tiles[j].tileLetter, other.positions[j]);
    return merged;
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

// Friend insertion operator: prints the whole board, same layout as displayBoard().
ostream& operator<<(ostream& os, const Scrabble& board) {
    for (int w = 0; w < 2; w++) {
        const DynamicArray<Tile>& r = (w == 0) ? board.rowOne : board.rowTwo;
        os << "  Word " << (w + 1) << ": ";
        for (int i = 0; i < r.getSize(); i++) os << r[i] << " ";
        if (board.isRowComplete(w == 0 ? WORD_1 : WORD_2)) os << " COMPLETE";
        os << endl;
    }
    return os;
}

// Friend extraction operator: reads letter, position and row number (1 or 2), then places it.
istream& operator>>(istream& is, Scrabble& board) {
    Clue c;
    int rowNumber;
    is >> c.clueLetter >> c.cluePosition >> rowNumber;
    if (rowNumber != 1 && rowNumber != 2) throw InvalidPositionException();
    c.clueRow = (rowNumber == 1) ? WORD_1 : WORD_2;
    board.placeLetter(c);
    return is;
}

// ---------- Cottage ----------
// Earlier raw DMA version (kept as a comment):
//   Location* locationList;  locationList = new Location[7];  ...  delete[] locationList;

// Reserves room for the seven locations of the cottage.
Cottage::Cottage() : locationList(7) {}

// Deep copy of all locations.
Cottage::Cottage(const Cottage& other) : locationList(other.locationList) {}

// Assignment operator performs a deep copy as well.
Cottage& Cottage::operator=(const Cottage& other) {
    if (this != &other) locationList = other.locationList;
    return *this;
}

// The DynamicArray destructor deletes the location array itself.
Cottage::~Cottage() {}

// Adds one location (name, letter, position) to the cottage.
void Cottage::addLocation(const Location& loc) { locationList.add(loc); }

int Cottage::getLocationCount() const { return locationList.getSize(); }

// Tells whether the location at index was searched earlier.
bool Cottage::isSearched(int index) const { return locationList[index].locSearchFlag; }

string Cottage::getLocationName(int index) const { return locationList[index].locName; }

// Marks the location as searched and returns its letter as a Clue.
Clue Cottage::searchLocation(int index) {
    Location& loc = locationList[index];
    loc.locSearchFlag = true;
    Clue c = { loc.letterClueLoc, loc.boardPosLoc, loc.locRow };
    return c;
}

// Prints all locations with their search status.
void Cottage::displayLocations() const {
    for (int i = 0; i < locationList.getSize(); i++) {
        cout << "[" << (i + 1) << "] " << locationList[i].locName << " - "
            << (locationList[i].locSearchFlag ? "Searched" : "NOT Searched") << endl;
    }
}

// ---------- ScoreBoard ----------

// Remembers which file stores the scores.
ScoreBoard::ScoreBoard(string file) : fileName(file) {}

// Appends one finished game to the file; creates the file with a header if it does not exist.
void ScoreBoard::addScore(string detective, string result, int credibility, int turnsLeft, string culprit) {
    bool needsNewline = false;
    bool fileExists = false;

    ifstream check(fileName.c_str(), ios::binary);
    if (check.is_open()) {
        fileExists = true;
        check.seekg(0, ios::end);
        if (check.tellg() > 0) {
            check.seekg(-1, ios::end);
            char last;
            check.get(last);
            needsNewline = (last != '\n');
        }
        check.close();
    }

    ofstream file(fileName.c_str(), ios::app);
    if (!file.is_open()) throw FileException(fileName);
    if (!fileExists) file << "Detective|Result|Credibility|Turns_Left|Culprit|Date" << endl;
    if (needsNewline) file << endl;
    file << detective << "|" << result << "|" << credibility << "|" << twoDigits(turnsLeft)
        << "|" << culprit << "|" << todayDate() << endl;
}

// Reads the file and prints every saved game; throws if the file is missing or corrupted.
void ScoreBoard::displayAll() const {
    ifstream file(fileName.c_str());
    if (!file.is_open()) throw FileException(fileName);

    string line;
    if (!getline(file, line)) throw FileException(fileName);

    cout << "Detective            Result  Cred  Turns Left  Culprit      Date" << endl;
    cout << "-----------------------------------------------------------------" << endl;

    int rows = 0;
    while (getline(file, line)) {
        if (trimText(line) == "") continue;
        string p[6];
        if (splitLine(line, p, 6) != 6) throw FileException(fileName);
        string cols[6] = { p[0], p[1], p[2], p[3], p[4], p[5] };
        int widths[6] = { 21, 8, 6, 12, 13, 8 };
        for (int i = 0; i < 6; i++) {
            string cell = cols[i];
            while ((int)cell.length() < widths[i]) cell += " ";
            cout << cell;
        }
        cout << endl;
        rows++;
    }
    if (rows == 0) cout << "(no games played yet)" << endl;
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

// Default clue (a blank letter); Victim and Suspect override it with their real letter.
Clue Character::getClue() const {
    Clue c = { ' ', 0, WORD_2 };
    return c;
}

// Friend insertion operator: prints name, role and nationality.
ostream& operator<<(ostream& os, const Character& c) {
    os << c.characterName << " (" << c.characterRole << ", " << c.nationality << ")";
    return os;
}

// Friend extraction operator: reads name, role and nationality (no spaces) from the keyboard.
istream& operator>>(istream& is, Character& c) {
    is >> c.characterName >> c.characterRole >> c.nationality;
    return is;
}

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

// ---------- GameEngine ----------

// The single object lives here; it is created the first time getInstance() is called.
GameEngine* GameEngine::instance = 0;

// Private constructor: sets default rules and prepares the random number generator.
GameEngine::GameEngine()
    : maxTurns(25), startCredibility(100), wrongTurnPenalty(5), wrongWordPenalty(10),
    winCondition("Accuse_Correct_Culprit"), loseCondition("Credibility_Zero_Or_Turns_Exhausted"),
    rulesFromFile(false), culpritIndex(0), culpritRole(""),
    detective(0), clueRack(16), peopleCount(0), scoreBoard(0), state(MENU) {
    srand((unsigned)time(0));
    for (int i = 0; i < 8; i++) { people[i] = 0; interviewed[i] = false; }
    wordSolved[0] = false;
    wordSolved[1] = false;
}

// Private destructor: frees the detective and all characters.
GameEngine::~GameEngine() { clearGame(); }

// Returns the only GameEngine; creates it on first use (Singleton pattern).
GameEngine& GameEngine::getInstance() {
    if (instance == 0) instance = new GameEngine();
    return *instance;
}

// Deletes the single object (called once at the end of main).
void GameEngine::destroyInstance() {
    delete instance;
    instance = 0;
}

// Frees every heap object of the previous game so there is no memory leak.
void GameEngine::clearGame() {
    for (int i = 0; i < 8; i++) {
        delete people[i];    // virtual destructor deletes the right derived object
        people[i] = 0;
    }
    peopleCount = 0;
    delete detective;
    detective = 0;
}

void GameEngine::setScoreBoard(ScoreBoard* sb) { scoreBoard = sb; }

// Reads rules.txt (Key=Value lines); falls back to the defaults if it is missing or corrupt.
void GameEngine::loadRules() {
    int turns = 25, credibility = 100, wrongTurn = 5, wrongWord = 10;
    string win = "Accuse_Correct_Culprit";
    string lose = "Credibility_Zero_Or_Turns_Exhausted";
    bool ok = true;

    ifstream file("rules.txt");
    if (!file.is_open()) ok = false;

    string line;
    int found = 0;
    while (ok && getline(file, line)) {
        line = trimText(line);
        if (line == "") continue;
        int eq = (int)line.find('=');
        if (eq <= 0) { ok = false; break; }
        string key = trimText(line.substr(0, eq));
        string value = trimText(line.substr(eq + 1));
        if (key == "Max_Turns") { turns = atoi(value.c_str()); found++; }
        else if (key == "Starting_Credibility") { credibility = atoi(value.c_str()); found++; }
        else if (key == "Every_Wrong_Turn") { wrongTurn = atoi(value.c_str()); found++; }
        else if (key == "Every_Wrong_Word") { wrongWord = atoi(value.c_str()); found++; }
        else if (key == "Win_Condition") { win = value; found++; }
        else if (key == "Lose_Condition") { lose = value; found++; }
    }
    if (found < 6 || turns <= 0 || credibility <= 0) ok = false;

    if (ok) {
        maxTurns = turns;
        startCredibility = credibility;
        wrongTurnPenalty = (wrongTurn < 0) ? -wrongTurn : wrongTurn;   // the file stores -5
        wrongWordPenalty = (wrongWord < 0) ? -wrongWord : wrongWord;
        winCondition = win;
        loseCondition = lose;
        rulesFromFile = true;
    }
    else {
        maxTurns = 25; startCredibility = 100; wrongTurnPenalty = 5; wrongWordPenalty = 10;
        winCondition = "Accuse_Correct_Culprit";
        loseCondition = "Credibility_Zero_Or_Turns_Exhausted";
        rulesFromFile = false;
    }
}

// Prints the rules that are currently in use.
void GameEngine::showRules() const {
    cout << "-------- RULES --------" << endl;
    if (!rulesFromFile) cout << "(rules.txt missing or corrupt - default rules are used)" << endl;
    cout << "Maximum turns        : " << maxTurns << endl;
    cout << "Starting credibility : " << startCredibility << endl;
    cout << "Every wrong turn     : -" << wrongTurnPenalty << endl;
    cout << "Every wrong word     : -" << wrongWordPenalty << endl;
    cout << "Win condition        : " << winCondition << endl;
    cout << "Lose condition       : " << loseCondition << endl;
}

// Prints about.txt; throws FileException if the file is missing.
void GameEngine::showAbout() const {
    ifstream file("about.txt");
    if (!file.is_open()) throw FileException("about.txt");
    string line;
    while (getline(file, line)) cout << line << endl;
}

// Reads clue.txt: role, Word 1 and Word 2 of the seven culprit profiles.
void GameEngine::loadClues(string roles[], string words1[], string words2[]) {
    string lines[7];
    if (readDataLines("clue.txt", lines, 7) != 7) throw FileException("clue.txt");
    for (int i = 0; i < 7; i++) {
        string f[3];
        if (splitLine(lines[i], f, 3) != 3) throw FileException("clue.txt");
        roles[i] = f[0];
        words1[i] = f[1];
        words2[i] = f[2];
        if (words1[i].length() != 7 || words2[i].length() != 8) throw FileException("clue.txt");
    }
}

// Reads locations.txt and builds the cottage; the letter column depends on the culprit profile.
void GameEngine::loadLocations(int profile) {
    string lines[7];
    if (readDataLines("locations.txt", lines, 7) != 7) throw FileException("locations.txt");

    Cottage fresh;
    for (int i = 0; i < 7; i++) {
        string f[9];
        if (splitLine(lines[i], f, 9) != 9) throw FileException("locations.txt");
        int position = atoi(f[1].c_str());
        string letterText = f[2 + profile];
        if (position < 1 || position > 7 || letterText.length() != 1) throw FileException("locations.txt");
        Location loc = { f[0], letterText[0], position, WORD_1, false };
        fresh.addLocation(loc);
    }
    cottage = fresh;   // uses the deep-copy assignment operator of Cottage
}

// Creates the right character object for a role name (a small factory).
static Character* createCharacter(string role, string name, string nation, char letter, int pos) {
    if (role == "Victim") return new Victim(name, nation, letter, pos);
    if (role == "Cook") return new Cook(name, nation, letter, pos);
    if (role == "Guard") return new Guard(name, nation, letter, pos);
    if (role == "Servant") return new Servant(name, nation, letter, pos);
    if (role == "Child") return new OwnersChild(name, nation, letter, pos);
    if (role == "Partner_1" || role == "Partner_2") return new Partner(name, role, nation, letter, pos);
    if (role == "PA") return new PA(name, nation, letter, pos);
    return 0;
}

// Reads characters.txt and creates the eight people; the letter column depends on the profile.
void GameEngine::loadCharacters(int profile) {
    string lines[8];
    if (readDataLines("characters.txt", lines, 8) != 8) throw FileException("characters.txt");

    Character* loaded[8];
    for (int i = 0; i < 8; i++) loaded[i] = 0;

    for (int i = 0; i < 8; i++) {
        string f[12];
        int fields = splitLine(lines[i], f, 12);
        int position = atoi(f[3].c_str());
        string letterText = (fields == 12) ? f[4 + profile] : "";
        if (fields != 12 || position < 1 || position > 8 || letterText.length() != 1) {
            for (int j = 0; j < i; j++) delete loaded[j];
            throw FileException("characters.txt");
        }
        loaded[i] = createCharacter(f[1], f[0], f[2], letterText[0], position);
        if (loaded[i] == 0) {
            for (int j = 0; j < i; j++) delete loaded[j];
            throw FileException("characters.txt");
        }
    }
    for (int i = 0; i < 8; i++) people[i] = loaded[i];
    peopleCount = 8;
}

// Prepares a fresh game: rules, random culprit, letters, board, cottage and detective.
// forcedCulprit (0 to 6) is only for testing; -1 means a random culprit.
void GameEngine::startNewGame(string detectiveName, int forcedCulprit) {
    clearGame();
    loadRules();

    string roles[7], words1[7], words2[7];
    loadClues(roles, words1, words2);

    culpritIndex = (forcedCulprit >= 0 && forcedCulprit < 7) ? forcedCulprit : rand() % 7;
    culpritRole = roles[culpritIndex];

    loadLocations(culpritIndex);
    loadCharacters(culpritIndex);

    // Friend access: the engine marks the culprit directly on the Character object.
    bool marked = false;
    for (int i = 0; i < peopleCount; i++) {
        if (people[i]->characterRole == culpritRole) {
            people[i]->isCulprit = true;
            marked = true;
        }
    }
    if (!marked) throw FileException("characters.txt");

    board = Scrabble();
    board.setTargetWords(words1[culpritIndex], words2[culpritIndex]);
    clueRack = TileRack(16);
    detective = new Detective(detectiveName, "Malay", startCredibility);
    for (int i = 0; i < 8; i++) interviewed[i] = false;
    wordSolved[0] = false;
    wordSolved[1] = false;
    state = PLAYING;
}

Detective& GameEngine::getDetective() { return *detective; }
Cottage& GameEngine::getCottage() { return cottage; }
Scrabble& GameEngine::getBoard() { return board; }
TileRack& GameEngine::getClueRack() { return clueRack; }
Character* GameEngine::getPerson(int index) { return people[index]; }
int GameEngine::getPeopleCount() const { return peopleCount; }
string GameEngine::getCulpritRole() const { return culpritRole; }
int GameEngine::getMaxTurns() const { return maxTurns; }
int GameEngine::getWrongTurnPenalty() const { return wrongTurnPenalty; }
int GameEngine::getWrongWordPenalty() const { return wrongWordPenalty; }
GameState GameEngine::getState() const { return state; }
void GameEngine::setState(GameState newState) { state = newState; }

// ---------- User interface helpers ----------

// Replaces every underscore with a space so "Bedroom_1" is shown as "Bedroom 1".
static string prettyName(string text) {
    for (int i = 0; i < (int)text.length(); i++) {
        if (text[i] == '_') text[i] = ' ';
    }
    return text;
}

// Prints a title inside a box made of simple characters (works in every console).
static void printHeader(string title) {
    const int width = 54;
    string line = "+";
    for (int i = 0; i < width; i++) line += "=";
    line += "+";
    int left = (width - (int)title.length()) / 2;
    if (left < 0) left = 0;
    string middle = "|";
    for (int i = 0; i < left; i++) middle += " ";
    middle += title;
    while ((int)middle.length() < width + 1) middle += " ";
    middle += "|";
    cout << line << endl << middle << endl << line << endl;
}

// Reads a whole line from the keyboard; if the input ends, the game is told to exit.
string GameEngine::readLine() {
    string line;
    if (!getline(cin, line)) {
        state = EXIT_GAME;
        throw InvalidInputException();
    }
    return trimText(line);
}

// Reads a number between low and high; anything else throws InvalidInputException.
int GameEngine::readChoice(int low, int high) {
    string line = readLine();
    if (line.empty() || line.length() > 3) throw InvalidInputException();
    int value = 0;
    for (int i = 0; i < (int)line.length(); i++) {
        if (line[i] < '0' || line[i] > '9') throw InvalidInputException();
        value = value * 10 + (line[i] - '0');
    }
    if (value < low || value > high) throw InvalidInputException();
    return value;
}

// Reads Y or N; any other answer throws InvalidInputException.
bool GameEngine::readYesNo() {
    string line = readLine();
    if (line == "Y" || line == "y") return true;
    if (line == "N" || line == "n") return false;
    throw InvalidInputException();
}

// Pauses until the player presses Enter, so messages are not cleared too early.
void GameEngine::waitForEnter() {
    cout << "Press Enter to continue...";
    string line;
    getline(cin, line);
}

// Clears the console screen (cls on Windows, clear on other systems).
void GameEngine::clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

// ---------- Main menu (Module 13) ----------

// Runs the game: the engine behaves according to its current state until the player exits.
// Exceptions are not caught here; main() catches them and calls run() again to resume.
void GameEngine::run() {
    while (state != EXIT_GAME) {
        if (state == MENU) mainMenuStep();
        else if (state == PLAYING) gameStep();
        else endGame();   // WON or LOST
    }
}

// Shows the main menu once and performs the chosen action.
void GameEngine::mainMenuStep() {
    clearScreen();
    printHeader("THE LOCKER HEIST - MAIN MENU");
    cout << "   [1] New Game" << endl;
    cout << "   [2] View Leader/Score Board" << endl;
    cout << "   [3] About" << endl;
    cout << "   [4] Rules" << endl;
    cout << "   [5] Exit" << endl;
    cout << "Choice: ";

    MenuChoice choice = (MenuChoice)readChoice(1, 5);
    cout << endl;

    if (choice == NEW_GAME) {
        setupNewGame();
    }
    else if (choice == LEADER_BOARD) {
        if (scoreBoard == 0) throw FileException("leader/scoreBoard.txt");
        printHeader("LEADER / SCORE BOARD");
        scoreBoard->displayAll();
        waitForEnter();
    }
    else if (choice == ABOUT) {
        showAbout();
        waitForEnter();
    }
    else if (choice == RULES) {
        loadRules();
        showRules();
        waitForEnter();
    }
    else {
        cout << "Goodbye, Detective!" << endl;
        state = EXIT_GAME;
    }
}

// Asks for the detective name, prepares a fresh game and shows the rules.
void GameEngine::setupNewGame() {
    printHeader("THE LOCKER HEIST - NEW GAME");
    cout << "Enter detective name (no spaces): ";
    string name = readLine();
    if (name.empty()) throw InvalidInputException();

    // A name with spaces is accepted if the player picks _ or . to replace them.
    if (name.find(' ') != string::npos) {
        cout << "Your name has spaces. Type _ or . to use instead of a space: ";
        string symbol = readLine();
        if (symbol != "_" && symbol != ".") throw InvalidInputException();
        for (int i = 0; i < (int)name.length(); i++) {
            if (name[i] == ' ') name[i] = symbol[0];
        }
    }

    startNewGame(name);   // random culprit
    cout << endl << "A culprit has been secretly chosen. The game begins!" << endl << endl;
    showRules();
    cout << endl;
    waitForEnter();
}

// ---------- Game menu (Module 13) ----------

// Prints credibility, turns, both racks and the Scrabble board.
void GameEngine::showStatus() const {
    cout << endl << "Detective: " << detective->getName()
        << " | Credibility: " << detective->getCredibility()
        << " | Turns used: " << detective->getTurns() << "/" << maxTurns << endl;
    cout << "Rack (Word 1): ";
    detective->getRack(WORD_1).display();
    cout << "Rack (Word 2): ";
    detective->getRack(WORD_2).display();
    cout << "Scrabble:" << endl << board;
}

// Ends the game as a loss when credibility is zero or the turns are used up.
void GameEngine::checkEndConditions() {
    if (state != PLAYING) return;
    if (detective->getCredibility() <= 0) {
        cout << endl << "Your credibility has run out!" << endl;
        state = LOST;
    }
    else if (detective->getTurns() >= maxTurns) {
        cout << endl << "You have used all your turns!" << endl;
        state = LOST;
    }
}

// Shows the game menu once and performs the chosen action.
void GameEngine::gameStep() {
    checkEndConditions();
    if (state != PLAYING) return;

    showStatus();
    cout << endl << "-------- Game Menu --------" << endl;
    cout << "[1] Visit a Location" << endl;
    cout << "[2] Interview a Person" << endl;
    cout << "[3] View Scrabble Board (free - no turn)" << endl;
    cout << "[4] Form a Word" << endl;
    cout << "[5] Accuse a Suspect" << endl;
    cout << "[6] Quit" << endl;
    cout << endl << "--- TURN " << (detective->getTurns() + 1) << " ---" << endl;
    cout << "Choice: ";

    int choice = readChoice(1, 6);
    cout << endl;

    if (choice == 1) visitLocation();
    else if (choice == 2) interviewPerson();
    else if (choice == 3) {
        cout << "Scrabble Board:" << endl << board;
        cout << "Rack (Word 1): ";
        detective->getRack(WORD_1).display();
        cout << "Rack (Word 2): ";
        detective->getRack(WORD_2).display();
    }
    else if (choice == 4) formWord();
    else if (choice == 5) accuseSuspect();
    else {
        cout << "You left the investigation. This game is not saved on the score board." << endl;
        state = MENU;
        waitForEnter();
    }
}

// ---------- Visiting locations ----------

// Lets the player pick a location; a first visit gives a letter, a repeat visit costs a wrong turn.
void GameEngine::visitLocation() {
    cout << "====== Location Menu ======" << endl;
    for (int i = 0; i < cottage.getLocationCount(); i++) {
        cout << "[" << (i + 1) << "] " << prettyName(cottage.getLocationName(i)) << " - "
            << (cottage.isSearched(i) ? "Searched" : "NOT Searched") << endl;
    }
    cout << "===========================" << endl;
    cout << "Pick: ";
    int index = readChoice(1, cottage.getLocationCount()) - 1;
    string name = prettyName(cottage.getLocationName(index));

    if (cottage.isSearched(index)) {
        cout << "You have already searched " << name << ". Search again? (Y/N): ";
        if (!readYesNo()) return;   // going back costs nothing
        Clue c = cottage.searchLocation(index);
        detective->useTurn();
        detective->loseCredibility(wrongTurnPenalty);
        cout << "Letter " << c.clueLetter << " is at position " << c.cluePosition << " of Word 1." << endl;
        cout << "That was a wasted turn: -" << wrongTurnPenalty << " credibility." << endl;
    }
    else {
        cout << "You have NOT searched " << name << " yet. Search? (Y/N): ";
        if (!readYesNo()) return;
        Clue c = cottage.searchLocation(index);
        detective->getRack(WORD_1).addTile(c.clueLetter, c.cluePosition);
        board.placeLetter(c);
        detective->useTurn();
        cout << "Letter " << c.clueLetter << " collected at position " << c.cluePosition << " of Word 1." << endl;
        cout << "Letter has been placed." << endl;
    }
}

// ---------- Interviewing people ----------

// Lets the player interview someone; the credibility limit comes from the object itself (polymorphism).
void GameEngine::interviewPerson() {
    cout << "====== Character Menu ======" << endl;
    for (int i = 0; i < peopleCount; i++) {
        cout << "[" << (i + 1) << "] " << people[i]->getName() << " (" << prettyName(people[i]->getRole())
            << ") - " << (interviewed[i] ? "Interviewed" : "NOT Interviewed") << endl;
    }
    cout << "============================" << endl;
    cout << "Pick: ";
    int index = readChoice(1, peopleCount) - 1;
    Character* person = people[index];

    // Too little credibility: the person refuses, the turn is wasted.
    if (detective->getCredibility() < person->getRequiredCredibility()) {
        int before = detective->getCredibility();
        detective->useTurn();
        detective->loseCredibility(wrongTurnPenalty);
        cout << person->getName() << " refuses to talk. Needs credibility " << person->getRequiredCredibility()
            << " (yours was " << before << ")." << endl;
        cout << "That was a wasted turn: -" << wrongTurnPenalty << " credibility." << endl;
        throw LowCredibilityException();
    }

    if (interviewed[index]) {
        cout << "You have already interviewed " << person->getName() << ". Interview again? (Y/N): ";
        if (!readYesNo()) return;
        Clue c = person->getClue();
        detective->useTurn();
        detective->loseCredibility(wrongTurnPenalty);
        cout << "Letter " << c.clueLetter << " is at position " << c.cluePosition << " of Word 2." << endl;
        cout << "That was a wasted turn: -" << wrongTurnPenalty << " credibility." << endl;
    }
    else {
        cout << "You have NOT interviewed " << person->getName() << " yet. Interview? (Y/N): ";
        if (!readYesNo()) return;
        Clue c = person->getClue();      // virtual call: Victim or Suspect gives its own letter
        detective->getRack(WORD_2).addTile(c.clueLetter, c.cluePosition);
        board.placeLetter(c);
        interviewed[index] = true;
        detective->useTurn();
        cout << "Letter " << c.clueLetter << " collected at position " << c.cluePosition << " of Word 2." << endl;
        cout << "Placed." << endl;
    }
}

// ---------- Forming words (Module 14) ----------

// The player types the word of a completed row; a wrong word costs credibility.
void GameEngine::formWord() {
    cout << "Which word do you want to form? [1] Word 1  [2] Word 2 : ";
    int pick = readChoice(1, 2);
    WordRow row = (pick == 1) ? WORD_1 : WORD_2;

    if (wordSolved[pick - 1]) {
        cout << "Word " << pick << " is already solved." << endl;
        return;
    }
    if (!board.isRowComplete(row)) throw InvalidWordException("This row is not complete yet.");

    cout << "Enter the word: ";
    string guess = readLine();
    if (guess.empty()) throw InvalidInputException();

    bool correct = board.checkWord(row, guess);   // throws if the length is wrong
    detective->useTurn();

    if (correct) {
        wordSolved[pick - 1] = true;
        cout << "Correct! Word " << pick << " is solved." << endl;
        if (wordSolved[0] && wordSolved[1]) revealCulprit();
    }
    else {
        detective->loseCredibility(wrongWordPenalty);
        cout << "Wrong word! -" << wrongWordPenalty << " credibility." << endl;
    }
}

// Shows what both words point to once both are solved.
void GameEngine::revealCulprit() {
    // Operator +: the two racks are merged into one clue with a space between them.
    clueRack = detective->getRack(WORD_1) + detective->getRack(WORD_2);

    cout << endl << "Both words solved:" << endl;
    cout << "  " << board.getWord(WORD_1) << " + " << board.getWord(WORD_2)
        << " = " << prettyName(culpritRole) << " is the culprit!" << endl;
    cout << "  Clue letters (merged racks): [" << clueRack.getLetters() << "]" << endl;
    for (int i = 0; i < peopleCount; i++) {
        if (people[i]->isCulprit) {
            cout << "  Culprit: ";
            people[i]->display();   // virtual call
        }
    }
    cout << "  Now use [5] Accuse a Suspect to name the culprit." << endl;
}

// ---------- Accusing (Module 14) ----------

// The player accuses one of the seven suspects; the right suspect wins the game.
void GameEngine::accuseSuspect() {
    cout << "====== Accuse a Suspect ======" << endl;
    for (int i = 1; i < peopleCount; i++) {   // index 0 is the victim, who is not a suspect
        cout << "[" << i << "] " << people[i]->getName() << " (" << prettyName(people[i]->getRole()) << ")" << endl;
    }
    cout << "==============================" << endl;
    cout << "Accuse: ";
    int index = readChoice(1, peopleCount - 1);
    Character* suspect = people[index];

    detective->useTurn();
    if (suspect->isCulprit) {
        cout << suspect->getName() << " is the culprit!" << endl;
        state = WON;
    }
    else {
        detective->loseCredibility(wrongTurnPenalty);
        cout << suspect->getName() << " is NOT the culprit. -" << wrongTurnPenalty << " credibility." << endl;
    }
}

// Shows the result, saves it to the score board and returns to the main menu.
void GameEngine::endGame() {
    bool won = (state == WON);
    string culpritName = "";
    for (int i = 0; i < peopleCount; i++) {
        if (people[i]->isCulprit) culpritName = people[i]->getName();
    }
    int turnsLeft = maxTurns - detective->getTurns();
    if (turnsLeft < 0) turnsLeft = 0;

    cout << endl << "+======================================================+" << endl;
    cout << "  " << detective->getName() << (won ? " WIN!" : " LOSE!") << endl;
    cout << "  Final credibility: " << detective->getCredibility()
        << " || Turns used: " << detective->getTurns()
        << " || Culprit: " << prettyName(culpritRole) << " " << culpritName << endl;
    cout << "+======================================================+" << endl;

    state = MENU;   // set first, so the game can continue even if saving fails
    if (scoreBoard == 0) throw FileException("leader/scoreBoard.txt");
    scoreBoard->addScore(detective->getName(), won ? "WIN" : "LOSE",
        detective->getCredibility(), turnsLeft, culpritRole);
    cout << "Result saved on the leader board." << endl;
    waitForEnter();
}