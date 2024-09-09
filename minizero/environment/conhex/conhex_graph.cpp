#include "conhex_graph.h"
#include "color_message.h"
#include "random.h"
#include "sgf_loader.h"
#include <algorithm>
#include <iostream>
#include <string>

namespace minizero::env::conhex {

using namespace minizero::utils;

Cell::Cell(int cell_id)
{
    this->cellId_ = cell_id;
    this->capturePlayer_ = Player::kPlayerNone; // who capture this
}
void Cell::addHole(int hole)
{
    this->holes_.push_back(std::make_pair(hole, Player::kPlayerNone));
}
Player Cell::capture(int hole, Player player) // who capture this
{
    for (int idx = 0; idx < static_cast<int>(this->holes_.size()); idx++) {
        if (this->holes_[idx].first == hole && this->holes_[idx].second == Player::kPlayerNone) {
            this->holes_[idx].second = player;
        }
    }
    if (this->capturePlayer_ != Player::kPlayerNone) return this->capturePlayer_;

    int p1 = 0;
    int p2 = 0;
    for (int idx = 0; idx < static_cast<int>(this->holes_.size()); idx++) {
        if (this->holes_[idx].second == Player::kPlayer1) p1++;
        if (this->holes_[idx].second == Player::kPlayer2) p2++;
    }
    // std::cout << "cellid:" << cellId_ << " " << p1 << " " << p2 << std::endl;
    if (this->holes_.size() >= 5) {
        if (p1 == 3) this->capturePlayer_ = Player::kPlayer1;
        if (p2 == 3) this->capturePlayer_ = Player::kPlayer2;
    } else if (this->holes_.size() == 3) {
        if (p1 == 2) this->capturePlayer_ = Player::kPlayer1;
        if (p2 == 2) this->capturePlayer_ = Player::kPlayer2;
    }

    return this->capturePlayer_;
}

int Cell::getCellId()
{
    return this->cellId_;
}

void Cell::reset()
{
    for (int idx = 0; idx < static_cast<int>(this->holes_.size()); idx++) {
        this->holes_[idx].second = Player::kPlayerNone;
    }
    this->capturePlayer_ = Player::kPlayerNone;
}

void ConHexGraph::reset()
{
    this->sz_.resize(board_size_ * board_size_ + 4, 0);  // +4 stands for top/left/right/bottom
    this->parent_.resize(board_size_ * board_size_ + 4); // +4 stands for top/left/right/bottom
    this->cacheBoard_.resize(board_size_ * board_size_);
    fill(this->cacheBoard_.begin(), this->cacheBoard_.end(), Player::kPlayerNone);

    // DSU reset
    for (int i = 0; i < board_size_ * board_size_ + 4; i++) {
        this->parent_[i] = i;
        this->sz_[i] = 0;
    }

    // cell reset
    for (Cell& cell : cell_) {
        cell.reset();
    }
}

ConHexGraph::ConHexGraph(int board_size)
{
    this->board_size_ = board_size;
    this->cellEdges_.resize(board_size_ * board_size_);
    this->mapping_.resize(board_size_ * board_size_, std::vector<int>()); // 0-board_size-1
    // need to be optimized

    this->cell_.resize(board_size_ * board_size_); // player
    this->cell_id_cnt_ = 0;                        // start from 0
    this->topId_ = board_size_ * board_size_ + 0;
    this->leftId_ = board_size_ * board_size_ + 1;
    this->rightId_ = board_size_ * board_size_ + 2;
    this->bottomId_ = board_size_ * board_size_ + 3;
    reset();
}
/**
 * @brief add Cell
 *
 * @param isTop     closed to the top
 * @param isLeft    closed to the left
 * @param isRight   closed to the right
 * @param isBottom  closed to the bottom
 * @return int      cell_id
 */
int ConHexGraph::addCell(std::vector<int> hole_idxs, bool isTop, bool isLeft, bool isRight, bool isBottom)
{
    int cell_id = cell_id_cnt_++;
    Cell newCell = Cell(cell_id);

    // add
    for (auto& hole_idx : hole_idxs) {
        mapping_[hole_idx].push_back(cell_id);
        newCell.addHole(hole_idx);
    }

    newCell.isTop_ = isTop;
    newCell.isLeft_ = isLeft;
    newCell.isRight_ = isRight;
    newCell.isBottom_ = isBottom;

    cell_[cell_id] = newCell;
    return cell_id;
}

int ConHexGraph::addOuterCell(int hole_idx_1, int hole_idx_2, int hole_idx_3, bool isTop, bool isLeft, bool isRight, bool isBottom)
{
    std::vector<int> hole_idxs({hole_idx_1, hole_idx_2, hole_idx_3});
    return addCell(hole_idxs, isTop, isLeft, isRight, isBottom);
}

int ConHexGraph::addInnerCell(int hole_idx_1, int hole_idx_2, int hole_idx_3, int hole_idx_4, int hole_idx_5, int hole_idx_6)
{
    std::vector<int> hole_idxs({hole_idx_1, hole_idx_2, hole_idx_3, hole_idx_4, hole_idx_5, hole_idx_6});
    return addCell(hole_idxs, false, false, false, false);
}

int ConHexGraph::addCenterCell(int hole_idx_1, int hole_idx_2, int hole_idx_3, int hole_idx_4, int hole_idx_5)
{
    std::vector<int> hole_idxs({hole_idx_1, hole_idx_2, hole_idx_3, hole_idx_4, hole_idx_5});
    return addCell(hole_idxs, false, false, false, false);
}

void ConHexGraph::cellInnerConnect(int from_cell_id, int to_cell_id)
{
    // same as Union in DSU
    int fa = find(from_cell_id), fb = find(to_cell_id);
    if (fa == fb) return; // already same
    if (this->sz_[fa] > this->sz_[fb]) std::swap(fa, fb);
    this->parent_[fb] = fa;
    this->sz_[fa] = this->sz_[fb];
}

void ConHexGraph::selfEdgeConnect()
{
    for (int hole_idx = 0; hole_idx < board_size_ * board_size_; hole_idx++) {
        if (mapping_[hole_idx].size() == 1) continue;
        if (mapping_[hole_idx].size() == 2) continue; // debug
        if (mapping_[hole_idx].size() == 3) {
            int combination[] = {mapping_[hole_idx][0], mapping_[hole_idx][1], mapping_[hole_idx][2]};
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    if (i == j) continue;
                    this->cellEdges_[combination[i]].insert(combination[j]);
                }
            }
        }
    }
}

int ConHexGraph::find(int index)
{
    if (this->parent_[index] == index) return index;
    return this->parent_[index] = find(this->parent_[index]);
}

std::vector<std::pair<int, Player>> ConHexGraph::getCellsCapturePlayerByHole(int hole_idx)
{
    std::vector<std::pair<int, Player>> ret;
    for (int cell_id : mapping_[hole_idx]) {
        ret.push_back(std::make_pair(cell_id, cell_[cell_id].capturePlayer_));
    }
    return ret;
}

bool ConHexGraph::cellIsCapturedByPlayer(int cell_id, Player player) const
{
    if (cell_id >= static_cast<int>(cell_.size())) return false;
    return cell_[cell_id].capturePlayer_ == player;
}

std::string ConHexGraph::toString() const
{
    auto color_player_1 = minizero::utils::TextColor::kBlue;
    auto color_player_2 = minizero::utils::TextColor::kRed;
    /***
     *
    []_______________________[]
     |o |  *  |  *  |  *  | o|
     | *o--o--o--o--o--o--o* |
     | /   |  *  |  *  |   \ |
     |-o*  o--o--o--o--o  *o-|
     | |  /   |  *  |   \  | |
     |*o--o*  o--o--o  *o--o*|
     | |  |   /  |  \   |  | |
     |-o* o--o*  o  *o--o *o-|
     | |  |  |  / \  |  |  | |
     |*o--o* o--ooo--o* o--o*|
     | |  |  |  \ /  |  |  | |
     |-o* o--o*  o  *o--o *o-|
     | |  |  \   |   /  |  | |
     |*o--o*  o--o--o  *o--o*|
     | |  \   |  *  |   /  | |
     |-o*  o--o--o--o--o  *o-|
     | \   |  *  |  *  |   / |
     | *o--o--o--o--o--o--o* |
     |o |  *  |  *  |  *  | o|
    []-----------------------[]
    */
    std::string pattern[] = {
        "   A B  C  D  E  F  G  H I",
        " []=======================[]",
        "  lo |  *  |  *  |  *  | ol",
        "1 l *o--o--o--o--o--o--o* l 1",
        "  l /   |  *  |  *  |   \\ l",
        "2 l-o*  o--o--o--o--o  *o-l 2",
        "  l |  /   |  *  |   \\  | l",
        "3 l*o--o*  o--o--o  *o--o*l 3",
        "  l |  |   /  |  \\   |  | l",
        "4 l-o* o--o*  o  *o--o *o-l 4",
        "  l |  |  |  /*\\  |  |  | l",
        "5 l*o--o* o--ooo--o* o--o*l 5",
        "  l |  |  |  \\*/  |  |  | l",
        "6 l-o* o--o*  o  *o--o *o-l 6",
        "  l |  |  \\   |   /  |  | l",
        "7 l*o--o*  o--o--o  *o--o*l 7",
        "  l |  \\   |  *  |   /  | l",
        "8 l-o*  o--o--o--o--o  *o-l 8",
        "  l \\   |  *  |  *  |   / l",
        "9 l *o--o--o--o--o--o--o* l 9",
        "  lo |  *  |  *  |  *  | ol",
        " []=======================[]",
        "   A B  C  D  E  F  G  H I"};
    bool debug_color = false;
    std::string colored_red_edge{minizero::utils::getColorText(
        "│", minizero::utils::TextType::kBold, minizero::utils::TextColor::kWhite,
        color_player_2)};
    std::string colored_blue_edge{minizero::utils::getColorText(
        "─", minizero::utils::TextType::kBold, minizero::utils::TextColor::kWhite,
        color_player_1)};

    std::string colored_blue_cell{minizero::utils::getColorText(
        " ", minizero::utils::TextType::kBold, minizero::utils::TextColor::kWhite,
        color_player_1)};

    std::string colored_red_cell{minizero::utils::getColorText(
        " ", minizero::utils::TextType::kBold, minizero::utils::TextColor::kWhite,
        color_player_2)};

    std::string colored_blue_node{minizero::utils::getColorText(
        "o", minizero::utils::TextType::kBold, color_player_1,
        minizero::utils::TextColor::kBlack)};

    std::string colored_red_node{minizero::utils::getColorText(
        "o", minizero::utils::TextType::kBold, color_player_2,
        minizero::utils::TextColor::kBlack)};
    if (debug_color) {
        colored_red_edge = "|";
        colored_blue_edge = "─";
        colored_blue_cell = "B";
        colored_red_cell = "R";
        colored_blue_node = "b";
        colored_red_node = "r";
    }
    int node_id_mapping[] = {0, 8, 1, 2, 3, 4, 5, 6, 7, 9, 11, 12, 13, 14, 15, 17, 18, 19, 21, 22, 23, 25, 26, 27, 28, 29, 31, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 49, 51, 52, 53, 54, 55, 57, 58, 59, 61, 62, 63, 65, 66, 67, 68, 69, 71, 73, 74, 75, 76, 77, 78, 79, 72, 80};
    int cell_id_mapping[] = {1, 2, 3,
                             0, 4,
                             17, 18,
                             16, 19,
                             29,
                             15, 28, 30, 5,
                             27, 36, 37, 20,
                             40,
                             14, 35, 31, 6,
                             40,
                             26, 39, 38, 21,
                             13, 34, 32, 7,
                             33,
                             25, 22,
                             24, 23,
                             12, 8,
                             11, 10, 9};
    std::string out;
    int cell_id_cnt = 0;
    int node_id_cnt = 0;
    for (std::string line : pattern) {
        for (char c : line) {
            if (c == '=') {
                out += colored_blue_edge;
            } else if (c == 'l') {
                out += colored_red_edge;
            } else if (c == '*') {
                int cell_id = cell_id_mapping[cell_id_cnt];
                if (cell_[cell_id].capturePlayer_ == Player::kPlayer1) out += colored_blue_cell;
                if (cell_[cell_id].capturePlayer_ == Player::kPlayer2) out += colored_red_cell;
                if (cell_[cell_id].capturePlayer_ == Player::kPlayerNone) out += " ";

                cell_id_cnt += 1;
            } else if (c == 'o') {
                int node_id = node_id_mapping[node_id_cnt];
                if (cacheBoard_[node_id] == Player::kPlayer1) out += colored_blue_node;
                if (cacheBoard_[node_id] == Player::kPlayer2) out += colored_red_node;
                if (cacheBoard_[node_id] == Player::kPlayerNone) out += "o";
                node_id_cnt += 1;
            } else {
                out += c;
            }
        }
        out += "\n";
    }
    return out; // dont show anything
}

/**
 * @brief
 *
 * @param hole_idx put on hole_idx
 * @return true  game ended, player win
 * @return false game not end
 */
Player ConHexGraph::cellCapture(int hole_idx, Player player)
{
    Player winner = Player::kPlayerNone;
    if (cacheBoard_[hole_idx] == Player::kPlayerNone) {
        cacheBoard_[hole_idx] = player;
    }
    for (int cell_id : mapping_[hole_idx]) {
        Player ret = cell_[cell_id].capture(hole_idx, player);
        if (ret == Player::kPlayerNone) continue; // no capture action happens

        // near edge or not
        if (ret == Player::kPlayer1 && cell_[cell_id].isTop_) {
            cellInnerConnect(cell_id, topId_);
        }
        if (ret == Player::kPlayer2 && cell_[cell_id].isLeft_) {
            cellInnerConnect(cell_id, leftId_);
        }
        if (ret == Player::kPlayer2 && cell_[cell_id].isRight_) {
            cellInnerConnect(cell_id, rightId_);
        }
        if (ret == Player::kPlayer1 && cell_[cell_id].isBottom_) {
            cellInnerConnect(cell_id, bottomId_);
        }

        for (int near_id : this->cellEdges_[cell_id]) {
            if (cell_[near_id].capturePlayer_ == ret) {
                cellInnerConnect(near_id, cell_id);
            }
        }
        if (player == Player::kPlayer1 && find(topId_) == find(bottomId_)) {
            winner = player;
        }
        if (player == Player::kPlayer2 && find(leftId_) == find(rightId_)) {
            winner = player;
        }
    }
    return winner;
}
} // namespace minizero::env::conhex
