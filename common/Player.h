//
// Created by devon on 5/6/15.
//

#ifndef MIDTERM_TICTACTOE_PLAYER_H
#define MIDTERM_TICTACTOE_PLAYER_H
#include <string>
using std::string;

// Mode used to determine whether player is ingame or not
enum player_modes { COMMAND, INGAME, LOGIN };

class Player {
public:
    // Constructor
    Player();
    Player(int socket);

    // Copy constructor
    Player(const Player & copy);

    // Assignment Op Overload
    Player * operator=(const Player & rhs);

    // Getters and Setters
    void SetName(string name);
    string GetName() const;
    int GetSocket() const;
    player_modes GetMode() const;
    void SetMode(player_modes mode);
    void SetRank(string rank);
    string GetRank() const;
    string GetUserName() const;
    void SetUserName(string username);
    void SetPassword(string password);
    string GetPassword() const;
    void SetStatus(int status);
    int GetStatus() const;
    void SetID(int id);
    int GetID() const;
    void SetScore(int score);
    int GetScore() const;

    // Equality operator overload
    bool operator==(const Player & other) const;

private:
    int m_socket;
    string m_name;
    string m_rank;
    string m_username;
    string m_password;
    int m_id;
    int m_status;
    int m_score;
    player_modes m_mode;
};


#endif //MIDTERM_TICTACTOE_PLAYER_H
