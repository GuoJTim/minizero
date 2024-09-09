#ifndef MINIZERO_ENVIRONMENT_CONHEX_CONHEX_GRAPH_H_
#define MINIZERO_ENVIRONMENT_CONHEX_CONHEX_GRAPH_H_
#include "base_env.h"
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace minizero::env::conhex {

class Cell {
    // each cell have 3/6 holes
public:
    Cell()
    {
    }
    Cell(int cell_id);
    void addHole(int hole);
    Player capture(int hole, Player player);
    int getCellId();
    bool isTop_, isLeft_, isRight_, isBottom_;
    Player capturePlayer_;
    void reset();

private:
    int cellId_;
    std::vector<std::pair<int, Player>> holes_;
};
class ConHexGraph {
public:
    ConHexGraph(int board_size);
    int addOuterCell(int hole_idx_1, int hole_idx_2, int hole_idx_3, bool isTop, bool isLeft, bool isRight, bool isBottom);
    int addInnerCell(int hole_idx_1, int hole_idx_2, int hole_idx_3, int hole_idx_4, int hole_idx_5, int hole_idx_6);
    int addCenterCell(int hole_idx_1, int hole_idx_2, int hole_idx_3, int hole_idx_4, int hole_idx_5);
    void cellInnerConnect(int from_cell_id, int to_cell_id); // DSU
    void selfEdgeConnect();
    void reset();
    std::vector<std::pair<int, Player>> getCellsCapturePlayerByHole(int hole_idx);
    bool cellIsCapturedByPlayer(int cell_id, Player player) const;
    Player cellCapture(int hole_idx, Player player);
    std::string toString() const;

private:
    int addCell(std::vector<int> hole_idxs, bool isTop, bool isLeft, bool isRight, bool isBottom);
    int find(int index);                    // DSU
    std::vector<Cell> cell_;                // player capture this cell
    std::vector<std::set<int>> cellEdges_;  // cell_id -> {cell_id}
    std::vector<int> parent_;               // DSU
    std::vector<int> sz_;                   // DSU
    std::vector<std::vector<int>> mapping_; // hole_idx -> {cell_id}
    std::vector<Player> cacheBoard_;
    int cell_id_cnt_;
    int board_size_;
    int topId_, leftId_, rightId_, bottomId_;
};

} // namespace minizero::env::conhex
#endif // MINIZERO_ENVIRONMENT_CONHEX_CONHEX_GRAPH_H_
