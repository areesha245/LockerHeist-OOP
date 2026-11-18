#include <iostream>
#include "heist.h"
using namespace std;

int main() {
    // One base-class pointer array shows runtime polymorphism.
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
        people[i]->display();   // correct version is chosen at runtime
        cout << "  Required credibility: " << people[i]->getRequiredCredibility() << endl;
    }

    for (int i = 0; i < 8; i++) delete people[i];   // virtual destructor frees each object
    return 0;
}