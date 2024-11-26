#include <Backend/Classes/Card.h>
#include <Backend/Classes/Deck.h>

#include <vector>

int main(){
    Card card("test card", 25, "what is this", "a test");

    std::vector<Card> cards = {card};
    Deck test("test", 1234, cards);

    test.listCards();
    return 0;
}