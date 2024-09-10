#include "conhex_graph_cell.h"

namespace minizero::env::conhex {

using namespace minizero::utils;

ConHexGraphCell::ConHexGraphCell(int cell_id, ConHexGraphCellType cell_type)
{
    cell_type_ = cell_type;
    cell_id_ = cell_id;
    capture_player_ = Player::kPlayerNone; // default player
}
void ConHexGraphCell::initHole(std::vector<int>& holes)
{
    for (int& hole : holes) {
        holes_.push_back(std::make_pair(hole, Player::kPlayerNone));
    }
}

void ConHexGraphCell::placeStone(int hole, Player player) // who capture this
{
    for (int idx = 0; idx < static_cast<int>(holes_.size()); ++idx) {
        if (holes_[idx].first == hole && holes_[idx].second == Player::kPlayerNone) {
            holes_[idx].second = player;
        }
    }
    if (capture_player_ != Player::kPlayerNone) return; // if already captured early return

    int player1_captured_count = 0;
    int player2_captured_count = 0;

    for (int idx = 0; idx < static_cast<int>(holes_.size()); ++idx) {
        if (holes_[idx].second == Player::kPlayer1) player1_captured_count++;
        if (holes_[idx].second == Player::kPlayer2) player2_captured_count++;
    }

    if (cell_type_ == ConHexGraphCellType::OUTER) {
        // OUTER = 3 holes , first player get 2 holes win the cell
        if (player1_captured_count == 2) capture_player_ = Player::kPlayer1;
        if (player2_captured_count == 2) capture_player_ = Player::kPlayer2;
    }

    if (cell_type_ == ConHexGraphCellType::INNER) {
        // OUTER = 6 holes , first player get 3 holes win the cell
        if (player1_captured_count == 3) capture_player_ = Player::kPlayer1;
        if (player2_captured_count == 3) capture_player_ = Player::kPlayer2;
    }

    if (cell_type_ == ConHexGraphCellType::CENTER) {
        // CENTER = 5 holes , first player get 3 holes win the cell
        if (player1_captured_count == 3) capture_player_ = Player::kPlayer1;
        if (player2_captured_count == 3) capture_player_ = Player::kPlayer2;
    }
}

void ConHexGraphCell::setEdgeFlag(ConHexGraphEdgeFlag cell_edge_flag)
{
    edge_flag_ = cell_edge_flag;
}

bool ConHexGraphCell::isEdgeFlag(ConHexGraphEdgeFlag edge_flag)
{
    return (bool)(edge_flag_ & edge_flag);
}

int ConHexGraphCell::getCellId()
{
    return cell_id_;
}

void ConHexGraphCell::reset()
{
    for (int idx = 0; idx < static_cast<int>(holes_.size()); ++idx) {
        holes_[idx].second = Player::kPlayerNone;
    }
    capture_player_ = Player::kPlayerNone;
}

Player ConHexGraphCell::getCapturedPlayer() const
{
    return capture_player_;
}

} // namespace minizero::env::conhex
