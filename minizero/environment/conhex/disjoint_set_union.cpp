#include "disjoint_set_union.h"

namespace minizero::env::conhex {

using namespace minizero::utils;

DisjointSetUnion::DisjointSetUnion(int board_size)
{
    board_size_ = board_size;
    reset();
}

void DisjointSetUnion::reset()
{
    dsu_size_.resize(board_size_ * board_size_ + 4, 0); // +4 stands for top/left/right/bottom
    parent_.resize(board_size_ * board_size_ + 4);      // +4 stands for top/left/right/bottom

    // DSU reset
    for (int i = 0; i < board_size_ * board_size_ + 4; ++i) {
        this->parent_[i] = i;
        this->dsu_size_[i] = 0;
    }
}

int DisjointSetUnion::find(int index)
{
    if (this->parent_[index] == index) return index;
    return this->parent_[index] = find(this->parent_[index]);
}

void DisjointSetUnion::connect(int from_cell_id, int to_cell_id)
{
    // same as Union in DSU
    int fa = find(from_cell_id), fb = find(to_cell_id);
    if (fa == fb) return; // already same
    if (this->dsu_size_[fa] > this->dsu_size_[fb]) std::swap(fa, fb);
    this->parent_[fb] = fa;
    this->dsu_size_[fa] = this->dsu_size_[fb];
}

} // namespace minizero::env::conhex
