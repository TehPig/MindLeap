#include <Backend/Classes/Card.hpp>
#include <Backend/Classes/Deck.hpp>
#include <Backend/Classes/User.hpp>

#include <Backend/Database/setup.hpp>

#include <iostream>
#include <vector>

int main(){
    std::cout << "Test" << std::endl;

    Database* db = Database::getInstance("app_data.db");
    db->initialize();
    std::cout << "Test2" << std::endl;

    const Card card("test card", 25, "what is this", "a test");
    const Card card2("test 2", 76, "another question", "answer here");

    const std::vector<Card> cards = {card, card2};
    const Deck test("test deck", 1234, cards);

    const std::vector<Deck> decks = {test};
    const User user("test user", 1234, decks);

    

    int user_created = user.createUser();
    if (user_created == false) std::cout << "Error creating user.\n";
    else std::cout << "User created successfully." << "\n";

    //int card_created = card.createCard();
    //if(card_created == false) std::cout << "Error creating card.\n";

    user.listDecks();
    std::cout << "==========================" << "\n";
    test.listCards();
    std::cout << "==========================" << "\n";
    std::cout << card.getName() << " " << card.getID() << "\n";

    return 0;
}