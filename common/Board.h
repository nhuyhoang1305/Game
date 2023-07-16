#ifndef MIDTERM_TICTACTOE_BOARD_H
#define MIDTERM_TICTACTOE_BOARD_H

#define MAX_ROWS 5
#define MAX_COLS 5

enum CellType {EXXES, OHS, BLANK};

class Board
{
public:
    // Constructor which type this player is
    Board();

    // The current player makes a move
    void PlayerMakeMove(int row, int col);

    // The other player makes a move
    void OtherMakeMove(int row, int col);

    // Check if the player won
    bool IsWon(int row, int col) const;

    // Check if the players tied
    bool IsDraw() const;

    // Check if the player lost
    bool IsLost(int row, int col) const;

    // Print out the board
    void DrawBoard() const;

    // Setter for the type
    void SetType(CellType type);

    // Returns whether that cell is a blank
    bool IsBlank(int row, int col) const;

private:
    // Check if a certain cell type won
    bool TypeIsWon(CellType type) const;

    bool TypeIsWon(CellType type, int row, int col) const;

    // Return back the character for a certain cell
    char PrintCell(CellType cell) const;

    // Check if a cell valid
    bool IsValid(int row, int col) const;

private:
    CellType m_type;
    CellType m_board[MAX_ROWS][MAX_COLS];
};


#endif //MIDTERM_TICTACTOE_BOARD_H
