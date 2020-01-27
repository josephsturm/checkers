/* -----------------------------------------------------------------------------
minmax.hh

Name: Joseph Sturm
Date: 01/27/2020
----------------------------------------------------------------------------- */

#ifndef MINMAX_HH
#define MINMAX_HH

////////////////////////////////////////////////////////////////////////////////

#include "engine/Board.hh"

#include <vector>

////////////////////////////////////////////////////////////////////////////////

class Node {
    Board m_state;
    std::vector<Node> m_children;
    float m_eval;

private:
    Node(const Board &state);
    void find_children(Color col);

private:
    friend class MinMax;
};

////////////////////////////////////////////////////////////////////////////////

class MinMax {
    Color m_playing_for;
    int m_search_depth;
    
public:
    MinMax(Color playing_for, int search_depth);
    Board best_move(const Board &state);

private:
    void expand(Node &current, int counter);
    Node &propagate(Node &current, bool maxing);
    static Node max_node(const Node &n1, const Node &n2);
    static Node min_node(const Node &n1, const Node &n2);
};

////////////////////////////////////////////////////////////////////////////////

#endif