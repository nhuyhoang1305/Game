#include <iostream>
#include "Board.h"

using std::cout;
using std::endl;

// Constructor which type this player is
Board::Board()
    : m_type(EXXES)
{
    // Initialize the board to all blanks
    for(int i = 0; i < MAX_ROWS; ++i)
    {
        for(int j = 0; j < MAX_COLS; ++j)
            m_board[i][j] = BLANK;
    }
}

// Setter for the type
void Board::SetType(CellType type)
{
    m_type = type;
}

// The current player makes a move
void Board::PlayerMakeMove(int row, int col)
{
    m_board[row][col] = m_type;
}

// The other player makes a move
void Board::OtherMakeMove(int row, int col)
{
    if(m_type == EXXES)
        m_board[row][col] = OHS;
    else
        m_board[row][col] = EXXES;
}

// Check if a certain cell type won
bool Board::TypeIsWon(CellType type) const
{
    bool won = false;

    // Check rows for winning
    for(int i = 0; i < MAX_ROWS; ++i)
    {
        if(m_board[i][0] == type && m_board[i][1] == type &&
           m_board[i][2] == type)
            won = true;
    }

    // Check columns for winning
    for(int i = 0; i < MAX_COLS; ++i)
    {
        if(m_board[0][i] == type && m_board[1][i] == type &&
           m_board[2][i] == type)
            won = true;
    }

    // Check diagonals for winning
    if(m_board[0][0] == type && m_board[1][1] == type &&
            m_board[2][2] == type)
        won = true;

    if(m_board[0][2] == type && m_board[1][1] == type &&
            m_board[2][0] == type)
        won = true;

    return won;
}

// Check if the player won
bool Board::IsWon() const
{
    return TypeIsWon(m_type);
}

// Check if the players tied
bool Board::IsDraw() const
{
    bool draw = true;

    // Check if every cell is a blank
    for(int i = 0; i < MAX_ROWS; ++i)
    {
        for(int j = 0; j < MAX_COLS; ++j)
        {
            if(m_board[i][j] == BLANK)
                draw = false;
        }
    }

    return draw;
}

// Check if the player lost
bool Board::IsLost() const
{
    bool lost = false;

    if(m_type == EXXES)
        lost = TypeIsWon(OHS);
    else
        lost = TypeIsWon(EXXES);

    return lost;
}

// Print out the board
void Board::DrawBoard() const
{
    cout << PrintCell(m_board[0][0]) << "|"
         << PrintCell(m_board[0][1]) << "|"
         << PrintCell(m_board[0][2]) << endl;

    cout << "-----" << endl;

    cout << PrintCell(m_board[1][0]) << "|"
         << PrintCell(m_board[1][1]) << "|"
         << PrintCell(m_board[1][2]) << endl;

    cout << "-----" << endl;

    cout << PrintCell(m_board[2][0]) << "|"
         << PrintCell(m_board[2][1]) << "|"
         << PrintCell(m_board[2][2]) << endl;
}

char Board::PrintCell(CellType cell) const
{
    char cellchar = ' ';

    if(cell == EXXES)
        cellchar = 'X';
    else if (cell == OHS)
        cellchar = 'O';

    return cellchar;
}
// Returns whether that cell is a blank
bool Board::IsBlank(int row, int col) const
{
    bool isblank = false;

    if(m_board[row][col] == BLANK)
        isblank = true;

    return isblank;
}