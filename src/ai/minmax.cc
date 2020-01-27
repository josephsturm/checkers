/* -----------------------------------------------------------------------------
minmax.cc

Name: Joseph Sturm
Date: 01/27/2020
----------------------------------------------------------------------------- */

#include "ai/minmax.hh"
#include "engine/board.hh"

#include <random>
#include <vector>

////////////////////////////////////////////////////////////////////////////////

float rand_int(int min, int max) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(min, max);
    return dist(gen);
}

////////////////////////////////////////////////////////////////////////////////

float evaluate(const Board &state) {

    // difference in piece count (higher better for black)
    float piece_diff = (float)state.get_black().count() -
                       (float)state.get_white().count();

    // difference in kings count (higher better for black)
    float kings_diff = (float)(state.get_black() & state.get_kings()).count() -
                       (float)(state.get_white() & state.get_kings()).count();

    return piece_diff + (0.25 * kings_diff);
}

////////////////////////////////////////////////////////////////////////////////

MinMax::MinMax(Color playing_for, int search_depth) {
    m_playing_for = playing_for;
    m_search_depth = search_depth;
}

////////////////////////////////////////////////////////////////////////////////

// Perform the MinMax algorithm to find AI's next move.
Board MinMax::best_move(const Board &state) {

    // init node from state
    Node root(state);

    // expand the tree to desired depth
    expand(root, m_search_depth);

    // propagate values up the tree
    if (m_playing_for == BLACK) {
        propagate(root, true).m_state;
    } else {
        propagate(root, false).m_state;
    }

    // get a list of all nodes with optimal value...
    std::vector<Node> candidates;
    for (auto &child : root.m_children) {
        if (child.m_eval == root.m_eval) {
            candidates.push_back(child);
        }
    }

    // ...and return one's state randomly
    return candidates[rand_int(0, candidates.size() - 1)].m_state;
}

////////////////////////////////////////////////////////////////////////////////

// Builds a MinMax tree (of Node: Board and evaluation).
void MinMax::expand(Node &current, int counter) {

    // terminate if depth limit is reached
    if (counter-- == 0) return;

    // expand the current node
    if (m_playing_for == BLACK) {
        current.find_children(BLACK);
    } else {
        current.find_children(WHITE);
    }

    // loop through all children of current node
    for (auto &child : current.m_children) {

        // don't expand leaf nodes
        if (current.m_children.size() == 0) {
            continue;
        }

        // expand all children of current node
        if (m_playing_for == BLACK) {
            child.find_children(WHITE);
        } else {
            child.find_children(BLACK);
        }

        // recursive call
        expand(child, counter);
    }
}

////////////////////////////////////////////////////////////////////////////////

// Propagates Board evaluations up a MinMax tree.
Node &MinMax::propagate(Node &current, bool maxing) {

    // exit condition: node is leaf node
    if (current.m_children.size() == 0) {
        return current;
    }

    // choose to maximize or minimize 
    switch (maxing) {
    case true:
        // evaluation starts low
        current.m_eval = -1e6;

        // call "propagate" on each child, set eval to highest
        for (auto &child : current.m_children) {
            current.m_eval = max_node(current, propagate(child, false)).m_eval;
        }

        return current;
    
    case false:
        // evaluation starts high
        current.m_eval = 1e6;

        // call "propagate" on each child, set eval to lowest
        for (auto &child : current.m_children) {
            current.m_eval = min_node(current, propagate(child, true)).m_eval;
        }

        return current;
    }
}

////////////////////////////////////////////////////////////////////////////////

// Given two MinMax nodes, returns node with higher evaluation.
Node MinMax::max_node(const Node &n1, const Node &n2) {
    if (n2.m_eval > n1.m_eval) return n2;
    return n1;
}

////////////////////////////////////////////////////////////////////////////////

// Given two MinMax nodes, returns node with lower evaluation.
Node MinMax::min_node(const Node &n1, const Node &n2) {
    if (n2.m_eval < n1.m_eval) return n2;
    return n1;
}

////////////////////////////////////////////////////////////////////////////////

Node::Node(const Board &state) {
    m_state = state;
    m_eval = evaluate(state);
}

////////////////////////////////////////////////////////////////////////////////

// Generates and stores all children of a MinMax node.
void Node::find_children(Color col) {

    // get child states for the right color
    std::vector<Board> child_states;
    if (col == BLACK) {
        child_states = m_state.get_black_actions();
    } else {
        child_states = m_state.get_white_actions();
    }

    // create child nodes for all child states
    for (const auto &state : child_states) {
        m_children.push_back(Node(state));
    }
}

////////////////////////////////////////////////////////////////////////////////