#ifndef MIDTERM_TICTACTOE_GAME_H
#define MIDTERM_TICTACTOE_GAME_H
#include <string>
#include "Player.h"

using std::string;

class Game {
public:
    // Constructor that specifies the socket of the first player
    Game(string name, Player player1);

    // Copy Constructor
    Game(const Game & copy);

    // Assignment op overload
    Game * operator=(const Game & rhs);

    // Returns true if this game has that player
    bool HasPlayer(const Player & player) const;

    // Gets the other player's id
    Player GetOtherPlayer(const Player & player) const;

    // Getters and setters
    Player GetPlayer1() const;
    void SetPlayer1(Player m_player1);
    Player GetPlayer2() const;
    void SetPlayer2(Player m_player2);
    string GetName() const;

    // Equality operator overload
    bool operator==(const Game & other) const;
    bool operator==(const string & other) const;

private:
    Player m_player1;
    Player m_player2;
    string m_name;
};


#endif //MIDTERM_TICTACTOE_GAME_H
