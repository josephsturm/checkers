/* -----------------------------------------------------------------------------
board.cc

Name: Joseph Sturm
Date: 01/27/2020
----------------------------------------------------------------------------- */

#include "engine/board.hh"
#include "ai/minmax.hh"

#include <bitset>
#include <vector>

////////////////////////////////////////////////////////////////////////////////

enum Direction {NW = 4, NE = 5, SW = -5, SE = -4};

////////////////////////////////////////////////////////////////////////////////

Position bit_mask(int sq) {
    Position mask = EMPTY_BOARD;
    mask.set(sq);
    return mask;
}

////////////////////////////////////////////////////////////////////////////////

const Position &Board::get_black() const {
    return m_black;
}

const Position &Board::get_white() const {
    return m_white;
}

const Position &Board::get_kings() const {
    return m_kings;
}

const History &Board::get_history() const {
    return m_history;
}

////////////////////////////////////////////////////////////////////////////////

Position Board::get_open_squares() const {
    return ~m_black & ~m_white & ON_BOARD;
}

////////////////////////////////////////////////////////////////////////////////

Actors Board::get_black_takers() const {
    Actors takers {};

    // fetch the previous action
    const Action prev = m_history.back();

    // black can't take after moving or promoting
    if (prev.color == BLACK && (prev.type == MOVE || prev.promoted)) {
        return takers;
    }

    // black can't take if white is making a second take
    if (prev.color == WHITE && prev.type == TAKE && 
        get_white_takers().any_action) {

        return takers;
    }

    // find pieces cornerwise to open squares
    const Position OPEN = get_open_squares();
    const Position NW_OPEN = (OPEN >> 4);
    const Position NE_OPEN = (OPEN >> 5);
    const Position SW_OPEN = (OPEN << 5);
    const Position SE_OPEN = (OPEN << 4);

    // find all black takers
    takers.nw = ((NW_OPEN & m_white) >> 4) & m_black;
    takers.ne = ((NE_OPEN & m_white) >> 5) & m_black;
    takers.sw = ((SW_OPEN & m_white) << 5) & m_black & m_kings;
    takers.se = ((SE_OPEN & m_white) << 4) & m_black & m_kings;

    // if previous action was a black take, restrict takers
    if (prev.color == BLACK && prev.type == TAKE) {
        const Position MASK = bit_mask(prev.dst);
        takers.nw &= MASK;
        takers.ne &= MASK;
        takers.sw &= MASK;
        takers.se &= MASK;
    }

    // set flag to determine if takes are possible
    if (takers.nw.any() || takers.ne.any() ||
        takers.sw.any() || takers.se.any()) {

        takers.any_action = true;
    }

    return takers;
}

////////////////////////////////////////////////////////////////////////////////

Actors Board::get_white_takers() const {
    Actors takers {};
    
    // fetch the previous action
    const Action prev = m_history.back();
    
    // white can't take after moving or promoting
    if (prev.color == WHITE && (prev.type == MOVE || prev.promoted)) {
        return takers;
    }
    
    // white can't take if black is making a second take
    if (prev.color == BLACK && prev.type == TAKE && 
        get_black_takers().any_action) {
            
        return takers;
    }
    
    // find pieces cornerwise to open squares
    const Position OPEN = get_open_squares();
    const Position NW_OPEN = (OPEN >> 4);
    const Position NE_OPEN = (OPEN >> 5);
    const Position SW_OPEN = (OPEN << 5);
    const Position SE_OPEN = (OPEN << 4);
    
    // calculate all white takers
    takers.nw = ((NW_OPEN & m_black) >> 4) & m_white & m_kings;
    takers.ne = ((NE_OPEN & m_black) >> 5) & m_white & m_kings;
    takers.sw = ((SW_OPEN & m_black) << 5) & m_white;
    takers.se = ((SE_OPEN & m_black) << 4) & m_white;
    
    // if previous action was a white take, restrict takers
    if (prev.color == WHITE && prev.type == TAKE) {
        const Position MASK = bit_mask(prev.dst);
        takers.nw &= MASK;
        takers.ne &= MASK;
        takers.sw &= MASK;
        takers.se &= MASK;
    }
    
    // set flag to determine if takes are possible
    if (takers.nw.any() || takers.ne.any() ||
        takers.sw.any() || takers.se.any()) {

        takers.any_action = true;
    }

    return takers;
}

////////////////////////////////////////////////////////////////////////////////

Actors Board::get_black_movers() const {
    Actors movers {};
    
    // fetch the previous action
    const Action prev = m_history.back();
    
    // black can't move after taking any action
    if (prev.color == BLACK) {
        return movers;
    }
    
    // no piece can move if a take is available
    if (get_black_takers().any_action) {
        return movers;
    }
    
    // calculate all black movers
    const Position OPEN = get_open_squares();
    movers.nw = (OPEN >> 4) & m_black;
    movers.ne = (OPEN >> 5) & m_black;
    movers.sw = (OPEN << 5) & m_black & m_kings;
    movers.se = (OPEN << 4) & m_black & m_kings;
    
    // determine whether any moves are possible
    if (movers.nw.any() || movers.ne.any() ||
        movers.sw.any() || movers.se.any()) {
            
        movers.any_action = true;
    }
    
    return movers;
}

////////////////////////////////////////////////////////////////////////////////

Actors Board::get_white_movers() const {
    Actors movers {};
    
    // fetch the previous action
    const Action prev = m_history.back();
    
    // white can't move after taking any action
    if (prev.color == WHITE) {
        return movers;
    }
    
    // no piece can move if a take is available
    if (get_white_takers().any_action) {
        return movers;
    }
    
    // calculate all white movers
    const Position OPEN = get_open_squares();
    movers.nw = (OPEN >> 4) & m_white & m_kings;
    movers.ne = (OPEN >> 5) & m_white & m_kings;
    movers.sw = (OPEN << 5) & m_white;
    movers.se = (OPEN << 4) & m_white;
    
    // determine whether any moves are possible
    if (movers.nw.any() || movers.ne.any() ||
        movers.sw.any() || movers.se.any()) {
            
        movers.any_action = true;
    }
    
    return movers;
}

////////////////////////////////////////////////////////////////////////////////

Square Board::get_square_info(int sq) const {
    Square info {};
    
    // determine color and promotion status
    info.is_black = m_black.test(sq);
    info.is_white = m_white.test(sq);
    info.is_kings = m_kings.test(sq);
    
    // calculate movers and takers based on color
    Actors movers;
    Actors takers;
    if (info.is_black) {
        movers = get_black_movers();
        takers = get_black_takers();
    } else if (info.is_white) {
        movers = get_white_movers();
        takers = get_white_takers();
    } else {
        return info;
    }
    
    // fill out PieceInfo struct
    info.move_nw = movers.nw.test(sq);
    info.move_ne = movers.ne.test(sq);
    info.move_sw = movers.sw.test(sq);
    info.move_se = movers.se.test(sq);
    info.take_nw = takers.nw.test(sq);
    info.take_ne = takers.ne.test(sq);
    info.take_sw = takers.sw.test(sq);
    info.take_se = takers.se.test(sq);
    
    return info;
}

////////////////////////////////////////////////////////////////////////////////

int Board::player_move(int sq, int dir, Square *info) {
    
    // if Square info not provided, calculate Square info
    if (info == nullptr) {
        Square calculated_info = get_square_info(sq);
        info = &calculated_info;
    }
    
    // determine whether specified move is possible
    bool move_possible;
    switch (dir) {
    case NW:
        move_possible = info->move_nw;
        break;
    case NE:
        move_possible = info->move_ne;
        break;
    case SW:
        move_possible = info->move_sw;
        break;
    case SE:
        move_possible = info->move_se;
        break;
    default:
        move_possible = false;
        break;
    }
    
    // if move is possble, move appropriate pieces
    if (move_possible) {
        if (info->is_black) move_black(sq, dir);
        if (info->is_white) move_white(sq, dir);
        if (info->is_kings) move_kings(sq, dir);
        return ACTION_SUCCESS;
    }
    
    return ACTION_FAILURE;
}

////////////////////////////////////////////////////////////////////////////////

int Board::player_take(int sq, int dir, Square *info) {
    
    // if Square info not provided, calculate Square info
    if (info == nullptr) {
        Square calculated_info = get_square_info(sq);
        info = &calculated_info;
    }
    
    // determine whether specified take is possible
    bool take_possible;
    switch (dir) {
    case NW:
        take_possible = info->take_nw;
        break;
    case NE:
        take_possible = info->take_ne;
        break;
    case SW:
        take_possible = info->take_sw;
        break;
    case SE:
        take_possible = info->take_se;
        break;
    default:
        take_possible = false;
        break;
    }
    
    // if take is possble, take appropriate pieces
    if (take_possible) {
        if (info->is_black) take_black(sq, dir);
        if (info->is_white) take_white(sq, dir);
        if (info->is_kings) take_kings(sq, dir);
        return ACTION_SUCCESS;
    }
    
    return ACTION_FAILURE;
}

////////////////////////////////////////////////////////////////////////////////

std::vector<Board> Board::get_black_actions() const {
    std::vector<Board> actions {};
    
    // fetch the previous action
    const Action prev = m_history.back();
    
    // short circuit if black just moved or promoted
    if (prev.color == BLACK && (prev.type == MOVE || prev.promoted)) {
        return actions;
    }
    
    // calculate movers and takers
    const Actors MOVERS = get_black_movers();
    const Actors TAKERS = get_black_takers();
    
    // calculate all actors
    const Position ALL_ACTORS =
        MOVERS.nw | MOVERS.ne | MOVERS.sw | MOVERS.se |
        TAKERS.nw | TAKERS.ne | TAKERS.sw | TAKERS.se;

    // check each square and add all actions to vector
    for (int sq = 0; sq < BOARD_SIZE; ++sq) {
        
        // if a square has no actor, skip it
        if (!ALL_ACTORS.test(sq)) {
            continue;
        }
        
        // get info for current square
        Square info = get_square_info(sq);
        
        // add legal actions to vector
        if (info.move_nw) {
            actions.push_back(*this);
            actions.back().player_move(sq, NW, &info);
        }
        
        if (info.move_ne) {
            actions.push_back(*this);
            actions.back().player_move(sq, NE, &info);
        }
        
        if (info.move_sw) {
            actions.push_back(*this);
            actions.back().player_move(sq, SW, &info);
        }
        
        if (info.move_se) {
            actions.push_back(*this);
            actions.back().player_move(sq, SE, &info);
        }
        
        if (info.take_nw) {
            actions.push_back(*this);
            actions.back().player_take(sq, NW, &info);
        }
        
        if (info.take_ne) {
            actions.push_back(*this);
            actions.back().player_take(sq, NE, &info);
        }
        
        if (info.take_sw) {
            actions.push_back(*this);
            actions.back().player_take(sq, SW, &info);
        }
        
        if (info.take_se) {
            actions.push_back(*this);
            actions.back().player_take(sq, SE, &info);
        }
    }
    
    return actions;
}

////////////////////////////////////////////////////////////////////////////////

std::vector<Board> Board::get_white_actions() const {
    std::vector<Board> actions {};
    
    // fetch the previous action
    const Action prev = m_history.back();
    
    // short circuit if white just moved or promoted
    if (prev.color == WHITE && (prev.type == MOVE || prev.promoted)) {
        return actions;
    }
    
    // calculate movers and takers
    const Actors MOVERS = get_white_movers();
    const Actors TAKERS = get_white_takers();
    
    // calculate all actors
    const Position ALL_ACTORS =
        MOVERS.nw | MOVERS.ne | MOVERS.sw | MOVERS.se |
        TAKERS.nw | TAKERS.ne | TAKERS.sw | TAKERS.se;
    
    // check each square and add all actions to vector
    for (int sq = 0; sq < BOARD_SIZE; ++sq) {
        
        // if a square has no actor, skip it
        if (!ALL_ACTORS.test(sq)) {
            continue;
        }
        
        // get info for current square
        Square info = get_square_info(sq);
        
        // add legal actions to vector
        if (info.move_nw) {
            actions.push_back(*this);
            actions.back().player_move(sq, NW, &info);
        }
        
        if (info.move_ne) {
            actions.push_back(*this);
            actions.back().player_move(sq, NE, &info);
        }
        
        if (info.move_sw) {
            actions.push_back(*this);
            actions.back().player_move(sq, SW, &info);
        }
        
        if (info.move_se) {
            actions.push_back(*this);
            actions.back().player_move(sq, SE, &info);
        }
        
        if (info.take_nw) {
            actions.push_back(*this);
            actions.back().player_take(sq, NW, &info);
        }
        
        if (info.take_ne) {
            actions.push_back(*this);
            actions.back().player_take(sq, NE, &info);
        }
        
        if (info.take_sw) {
            actions.push_back(*this);
            actions.back().player_take(sq, SW, &info);
        }
        
        if (info.take_se) {
            actions.push_back(*this);
            actions.back().player_take(sq, SE, &info);
        }
    }
    
    return actions;
}

////////////////////////////////////////////////////////////////////////////////

Board Board::ai_black_action(int depth) const {
    MinMax computer(BLACK, depth);
    return computer.best_move(*this);
}

////////////////////////////////////////////////////////////////////////////////

Board Board::ai_white_action(int depth) const {
    MinMax computer(WHITE, depth);
    return computer.best_move(*this);
}

////////////////////////////////////////////////////////////////////////////////

void Board::move_black(int sq, int dir) {
    Action move;
    
    // set color and action type
    move.color = BLACK;
    move.type = MOVE;
    
    // set source and destination squares
    move.src = sq;
    move.dst = sq + dir;
    
    // set whether this action results in a promotion
    move.promoted = !m_kings.test(move.src) && TOP_ROW.test(move.dst);
    
    // perform the move
    m_black.reset(move.src);
    m_black.set(move.dst);
    
    // update kings if piece lands in top row
    m_kings = m_kings | (m_black & TOP_ROW);
    
    // add move to board history
    this->m_history.push_back(move);
}

////////////////////////////////////////////////////////////////////////////////

void Board::move_white(int sq, int dir) {
    Action move;
    
    // set color and action type
    move.color = WHITE;
    move.type = MOVE;
    
    // set source and destination squares
    move.src = sq;
    move.dst = sq + dir;
    
    // set whether this action results in a promotion
    move.promoted = !m_kings.test(move.src) && BOT_ROW.test(move.dst);
    
    // perform the move
    m_white.reset(move.src);
    m_black.set(move.dst);
    
    // update kings if piece lands in bot row
    m_kings = m_kings | (m_white & BOT_ROW);
    
    // add move to board history
    this->m_history.push_back(move);
}

////////////////////////////////////////////////////////////////////////////////

void Board::move_kings(int sq, int dir) {
    
    // perform the move
    m_kings.reset(sq);
    m_kings.set(sq + dir);
}

////////////////////////////////////////////////////////////////////////////////

void Board::take_black(int sq, int dir) {
    Action take;
    
    // set color and action type
    take.color = BLACK;
    take.type = TAKE;
    
    // set source, destination, captured squares
    take.src = sq;
    take.dst = sq + (2 * dir);
    const int CAPTURED = sq + dir;
    
    // set whether this action results in a promotion
    take.promoted = !m_kings.test(take.src) && TOP_ROW.test(take.dst);
    
    // perform the take
    m_black.reset(take.src);
    m_white.reset(CAPTURED);
    m_black.set(take.dst);
    
    // update kings if piece lands in top row
    m_kings = m_kings | (m_black & TOP_ROW);
    
    // add take to board history
    this->m_history.push_back(take);
}

////////////////////////////////////////////////////////////////////////////////

void Board::take_white(int sq, int dir) {
    Action take;
    
    // set color and action type
    take.color = WHITE;
    take.type = TAKE;
    
    // set source, destination, captured squares
    take.src = sq;
    take.dst = sq + (2 * dir);
    const int CAPTURED = sq + dir;
    
    // set whether this action results in a promotion
    take.promoted = !m_kings.test(take.src) && BOT_ROW.test(take.dst);
    
    // perform the take
    m_white.reset(take.src);
    m_black.reset(CAPTURED);
    m_white.set(take.dst);
    
    // update kings if piece lands in top row
    m_kings = m_kings | (m_black & BOT_ROW);
    
    // add take to board history
    this->m_history.push_back(take);
}

////////////////////////////////////////////////////////////////////////////////

void Board::take_kings(int sq, int dir) {
    
    // perform the take
    m_kings.reset(sq);
    m_kings.set(sq + (2 * dir));
}

////////////////////////////////////////////////////////////////////////////////

int main() {}