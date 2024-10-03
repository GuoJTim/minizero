#pragma once

#include "base_env.h"
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace minizero::env::conhex {

class DisjointSetUnion {
public:
    DisjointSetUnion(int board_size);
    int find(int index);                            // DSU
    void connect(int from_cell_id, int to_cell_id); // DSU
    void reset();

private:
    std::vector<int> parent_;
    std::vector<int> dsu_size_;
    int board_size_;
};

} // namespace minizero::env::conhex
