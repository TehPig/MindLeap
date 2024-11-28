#include <Backend/Classes/User.hpp>
#include <Backend/Database/setup.hpp>

#include <iostream>
#include <algorithm>
#include <vector>
#include <string>

User::User(const std::string &u, int i, const std::vector<Deck> &d) : username(u), id(i), decks(d) {}

// Getters
void User::listDecks() const {
    for (int i = 0; i < this->decks.size(); i++){
        std::cout << i << ". " << decks[i].getName() << " - " << decks[i].getID() << "\n";
    }
}

// Setters
void User::selectUser(int id) {
    this->selected = this;
    std::cout << "Selected User: " << this->username << "\n";
}

// If decks are provided
bool User::createUser() const {
    Database* db = Database::getInstance("app_data.db");
    std::string query = "INSERT INTO Users (username, times_seen, last_viewed, next_review) VALUES ('" + this->username + "', 0, 0, 0);";
    if (!db->execute(query)) return false;

    // Get the user id of the newly created user
    int user_id = sqlite3_last_insert_rowid(db->getDB());

    for (const auto &deck : this->decks) {
        std::string deck_query = "INSERT INTO Decks (name, user_id) VALUES ('" + deck.getName() + "', " + std::to_string(user_id) + ");";
        if (!db->execute(deck_query)) return false;
    }

    return true;
}

// bool User::renameUser(int id, std::string username) {
//
// }
//
// bool User::deleteUser(int id) {
//
// }

void User::addDeck(const Deck &deck){
    decks.push_back(deck);
}

bool User::removeDeck(const Deck &deck){
    const auto item = find(this->decks.begin(), this->decks.end(), deck);
    if (item == this->decks.end()) return false;

    this->decks.erase(item);
    return true;
};