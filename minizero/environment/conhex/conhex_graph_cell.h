#ifndef MINIZERO_ENVIRONMENT_CONHEX_CONHEX_GRAPH_CELL_H_
#define MINIZERO_ENVIRONMENT_CONHEX_CONHEX_GRAPH_CELL_H_
#include "base_env.h"
#include "conhex_graph_flag.h"
#include <set>
#include <string>
#include <utility>
#include <vector>
namespace minizero::env::conhex {

class ConHexGraphCell {
public:
    ConHexGraphCell() = default;
    ConHexGraphCell(int cell_id, ConHexGraphCellType cell_type);

    void initHole(std::vector<int>& holes);
    int getCellId();

    Player getCapturedPlayer() const;
    void placeStone(int hole_id, Player player);
    void setEdgeFlag(ConHexGraphEdgeFlag cell_edge_flag);
    bool isEdgeFlag(ConHexGraphEdgeFlag cell_edge_flag);
    void reset();

private:
    ConHexGraphEdgeFlag edge_flag_;
    ConHexGraphCellType cell_type_;
    Player capture_player_;
    std::vector<std::pair<int, Player>> holes_;
    int cell_id_;
};

} // namespace minizero::env::conhex
#endif // MINIZERO_ENVIRONMENT_CONHEX_CONHEX_GRAPH_CELL_H_
