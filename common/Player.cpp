#include "Player.h"

// Constructor
Player::Player()
    : m_mode(COMMAND)
{ }
Player::Player(int socket)
    : m_socket(socket), m_name("No Name"), m_mode(COMMAND)
{ }

// Copy constructor
Player::Player(const Player &copy)
    : m_socket(-1), m_name("No Name"), m_mode(COMMAND)
{
    *this = copy;
}

// Assignment op overload
Player * Player::operator=(const Player & rhs)
{
    if(&rhs != this)
    {
        m_socket = rhs.m_socket;
        m_name = rhs.m_name;
        m_mode = rhs.m_mode;
    }

    return this;
}


// Getters and Setters
void Player::SetName(string name)
{
    m_name = name;
}
string Player::GetName() const
{
    return m_name;
}

void Player::SetSocket(int socket)
{
    m_socket = socket;
}

int Player::GetSocket() const
{
    return m_socket;
}

player_modes Player::GetMode() const
{
    return m_mode;
}

void Player::SetMode(player_modes mode)
{
    m_mode = mode;
}

void Player::SetRank(string rank)
{
    m_rank = rank;
}
string Player::GetRank() const
{

    if (m_score <= 50)
    {
        return "So cap";
    }
    else if (m_score <= 150)
    {
        return "Tap su";
    }
    else if (m_score <= 500)
    {
        return "Cao thu";
    }
    else
    {
        return "Ky thu";
    }

    return m_rank;
}
string Player::GetUserName() const
{
    return m_username;
}
void Player::SetUserName(string username)
{
    m_username = username;
}
void Player::SetPassword(string password)
{
    m_password = password;
}
string Player::GetPassword() const
{
    return m_password;
}
void Player::SetStatus(int status)
{
    m_status = status;
}
int Player::GetStatus() const
{
    return m_status;
}
void Player::SetID(int id)
{
    m_id = id;
}
int Player::GetID() const
{
    return m_id;
}
void Player::SetScore(int score)
{
    m_score = score;
}
int Player::GetScore() const
{
    return m_score;
}

// Equality operator overload
bool Player::operator==(const Player & other) const
{
    return m_name == other.m_name;
}

string Player::ToString() const
{
    string player = "";
    player += "Username: " + this->m_username + "\n";
    player += "Score: " + std::to_string(this->m_score) + "\n";
    player += "Rank: " + this->GetRank() + "\n";
    
    return player;
}