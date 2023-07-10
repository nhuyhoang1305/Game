
#include "Game.h"

/**
 * Constructor to define game with a name and starting player
 */
Game::Game(string name, Player player1)
    : m_player1(player1), m_player2(-1), m_name(name)
{ }

// Copy Constructor
Game::Game(const Game & copy)
    : m_player1(-1), m_player2(-1), m_name("No Name")
{
    *this = copy;
}

// Assignment op overload
Game *Game::operator=(const Game & rhs)
{
    if(&rhs != this)
    {
        m_player1 = rhs.m_player1;
        m_player2 = rhs.m_player2;
        m_name = rhs.m_name;
    }

    return this;
}


// Returns true if this game has that player
bool Game::HasPlayer(const Player & player) const
{
    bool hasplayer = false;

    if(m_player1 == player || m_player2 == player)
        hasplayer = true;

    return hasplayer;
}

/**
 * Gets the other player's id
 *
 * @param   player  The ID of the first player
 *
 * @return  Either the other player's ID, or -1
 */
Player Game::GetOtherPlayer(const Player & player) const
{
    Player other(-1);

    if(player == m_player1)
        other = m_player2;
    else if (player == m_player2)
        other = m_player1;

    return other;
}

/**
 * Getters and Setters
 */
Player Game::GetPlayer1() const {
    return m_player1;
}

void Game::SetPlayer1(Player m_player1) {
    Game::m_player1 = m_player1;
}

Player Game::GetPlayer2() const {
    return m_player2;
}

void Game::SetPlayer2(Player m_player2) {
    Game::m_player2 = m_player2;
}

string Game::GetName() const {
    return m_name;
}

// Equality operator overload
bool Game::operator==(const Game & other) const
{
    return m_name == other.m_name;
}

bool Game::operator==(const string & other) const
{
    return other == m_name;
}