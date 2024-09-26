#ifndef MINIZERO_ENVIRONMENT_CONHEX_CONHEX_GRAPH_H_
#define MINIZERO_ENVIRONMENT_CONHEX_CONHEX_GRAPH_H_
#include "base_env.h"
#include "conhex_graph_cell.h"
#include "conhex_graph_flag.h"
#include "disjoint_set_union.h"
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace minizero::env::conhex {

class ConHexGraph {
public:
    ConHexGraph(int board_size);

    bool isCellCapturedByPlayer(int cell_id, Player player) const;
    void placeStone(int hole_idx, Player player);
    Player getPlayerAtPos(int hole_idx) const;
    Player checkWinner() const;
    void reset();
    std::string toString() const;

private:
    void initGraph();
    void addCell(std::vector<int> hole_indexes, ConHexGraphEdgeFlag cell_edge_flag);

    DisjointSetUnion graph_dsu_;
    std::vector<std::vector<int>> hole_to_cell_map_; // hole_idx* -> cell_id, on same hole id may have many cell
    std::vector<std::set<int>> cell_adjacency_list_;  // cell_id -> cell_id* , adj list

    std::vector<ConHexGraphCell> cell_list_;
    std::vector<Player> board_;
    Player winner_;
    int cell_id_cnt_;

    const int board_size_;
    const int top_id_ = board_size_ * board_size_;
    const int left_id_ = board_size_ * board_size_ + 1;
    const int right_id_ = board_size_ * board_size_ + 2;
    const int bottom_id_ = board_size_ * board_size_ + 3; // change to static variable this is not a variable that need to change
};

} // namespace minizero::env::conhex
#endif // MINIZERO_ENVIRONMENT_CONHEX_CONHEX_GRAPH_H_
