#include <iostream>
#include "heist.h"
using namespace std;

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
Detective::Detective(string name, string nation, int startCredibility)
    : Character(name, "Detective", nation), credibility(startCredibility), turns(0) {
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