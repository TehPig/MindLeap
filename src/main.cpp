#include <Backend/Classes/Card.h>
#include <Backend/Classes/Deck.h>
#include <Backend/Classes/User.h>

#include <iostream>
#include <vector>

int main(){
    const Card card("test card", 25, "what is this", "a test");
    const Card card2("test 2", 76, "another question", "answer here");

    const std::vector<Card> cards = {card, card2};
    Deck test("test deck", 1234, cards);

    const std::vector<Deck> decks = {test};
    User user("test user", 1234, decks);

    user.listDecks();
    std::cout << "==========================" << "\n";
    test.listCards();
    std::cout << "==========================" << "\n";
    std::cout << card.getName() << " " << card.getID() << "\n";
    return 0;
}