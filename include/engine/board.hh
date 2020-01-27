/* -----------------------------------------------------------------------------
board.hh

Provides tools to represent and manipulate a checkers board by:
    1. implementing a data structure to indicate piece positions (Position)
    2. implementing a data structure to hold a history of actions (History)
    3. defining "Board" class as three Positions (B, W, K) and a History
    4. providing methods to observe and manipulate the Board

Position notation:
    xx 41 xx 42 xx 43 xx 44 xx 45
    36 -- 37 -- 38 -- 39 -- 40 xx
    xx 32 -- 33 -- 34 -- 35 -- 36
    27 -- 28 -- 29 -- 30 -- 31 xx
    xx 23 -- 24 -- 25 -- 26 -- 27
    18 -- 19 -- 20 -- 21 -- 22 xx
    xx 14 -- 15 -- 16 -- 17 -- 18
    09 -- 10 -- 11 -- 12 -- 13 xx
    xx 05 -- 06 -- 07 -- 08 -- 09
    00 xx 01 xx 02 xx 03 xx 04 xx

Name: Joseph Sturm
Date: 01/27/2020
----------------------------------------------------------------------------- */

#ifndef ENGINE_HH
#define ENGINE_HH

////////////////////////////////////////////////////////////////////////////////

#include <bitset>
#include <vector>

////////////////////////////////////////////////////////////////////////////////

#define ACTION_SUCCESS 0
#define ACTION_FAILURE 1

////////////////////////////////////////////////////////////////////////////////

// A total of 46 squares must be represented.
const auto BOARD_SIZE {46};

////////////////////////////////////////////////////////////////////////////////

// Piece locations are stored using bitsets.
using Position = std::bitset<BOARD_SIZE>;

// Promotion squares for black, white pieces.
const Position TOP_ROW {0x1E000000000};
const Position BOT_ROW {0x000000001E0};

// The set of all squares a piece can occupy.
const Position ON_BOARD {0x1EFF7FBFDE0};

// Starting squares for black, white, king pieces (empty).
const Position BLACK_START {0x0000003FDE0};
const Position WHITE_START {0x1EFF0000000};
const Position EMPTY_BOARD {0x00000000000};

////////////////////////////////////////////////////////////////////////////////

// Stores the set of all actors (of a certain type).
struct Actors {
    Position nw {EMPTY_BOARD};
    Position ne {EMPTY_BOARD};
    Position sw {EMPTY_BOARD};
    Position se {EMPTY_BOARD};
    bool any_action;
};

////////////////////////////////////////////////////////////////////////////////

// Stores all information about a specific square.
struct Square {
    bool is_black, is_white, is_kings;
    bool move_nw, move_ne, move_sw, move_se;
    bool take_nw, take_ne, take_sw, take_se;
};

////////////////////////////////////////////////////////////////////////////////

// A player can play black or white pieces.
enum Color {BLACK, WHITE};

// The types of action a player can take.
enum Type {NONE, MOVE, TAKE};

// Fully represents one action taken during a turn.
struct Action {
    Color color;
    Type type;
    int src, dst;
    bool promoted;
    // TODO: possibly store a Board here?
};

// The board "History" is every past action.
using History = std::vector<Action>;

////////////////////////////////////////////////////////////////////////////////

class Board {
    // piece locations
    Position m_black = BLACK_START;
    Position m_white = WHITE_START;
    Position m_kings = EMPTY_BOARD;

    // board history
    History m_history;
    
public:
    // the player chooses an action
    int player_move(int sq, int dir, Square *info = nullptr);
    int player_take(int sq, int dir, Square *info = nullptr);
    
    // the AI to chooses an action
    Board ai_black_action(int depth) const;
    Board ai_white_action(int depth) const;
    
    // calculates info about a square
    Square get_square_info(int sq) const;

    // gets a list of possible actions
    std::vector<Board> get_black_actions() const;
    std::vector<Board> get_white_actions() const;

    // get position of pieces
    const Position &get_black() const;
    const Position &get_white() const;
    const Position &get_kings() const;
    
    // get board history
    const History &get_history() const;

private:
    // finds all open squares
    Position get_open_squares() const;

    // finds all pieces that can move
    Actors get_black_movers() const;
    Actors get_white_movers() const;

    // finds all pieces that can take
    Actors get_black_takers() const;
    Actors get_white_takers() const;

private:
    // writes moves to board
    void move_black(int sq, int dir);    
    void move_white(int sq, int dir);
    void move_kings(int sq, int dir);
    
    // writes takes to board
    void take_black(int sq, int dir);
    void take_white(int sq, int dir);
    void take_kings(int sq, int dir);
};

////////////////////////////////////////////////////////////////////////////////

#endif