#include "conhex.h"
#include "color_message.h"
#include "random.h"
#include "sgf_loader.h"
#include <algorithm>
#include <iostream>
#include <string>

namespace minizero::env::conhex {

using namespace minizero::utils;

ConHexEnv::ConHexEnv() : BaseBoardEnv<ConHexAction>(kConHexBoardSize), conHexGraph_(kConHexBoardSize)
{
    // initial the cell network

    // goal
    // 1. get near hole faster in O(1)
    // 2. cell near cell in O(1)
    // 3. connect from edge to edge in O(1)

    // as graph , connect to each others
    // maintain two graph
    // one is used for connect near node
    // another one is used for detecting winning/connecting by using DSU
    this->conHexGraph_.addOuterCell(0, 1, 9, true, true, false, false);
    this->conHexGraph_.addOuterCell(1, 2, 3, true, false, false, false);
    this->conHexGraph_.addOuterCell(3, 4, 5, true, false, false, false);
    this->conHexGraph_.addOuterCell(5, 6, 7, true, false, false, false);
    this->conHexGraph_.addOuterCell(7, 8, 17, true, false, true, false);
    this->conHexGraph_.addOuterCell(17, 26, 35, false, false, true, false);
    this->conHexGraph_.addOuterCell(35, 44, 53, false, false, true, false);
    this->conHexGraph_.addOuterCell(53, 62, 71, false, false, true, false);
    this->conHexGraph_.addOuterCell(71, 79, 80, false, false, true, true);
    this->conHexGraph_.addOuterCell(77, 78, 79, false, false, false, true);
    this->conHexGraph_.addOuterCell(75, 76, 77, false, false, false, true);
    this->conHexGraph_.addOuterCell(73, 74, 75, false, false, false, true);
    this->conHexGraph_.addOuterCell(63, 72, 73, false, true, false, true);
    this->conHexGraph_.addOuterCell(45, 54, 63, false, true, false, false);
    this->conHexGraph_.addOuterCell(27, 36, 45, false, true, false, false);
    this->conHexGraph_.addOuterCell(9, 18, 27, false, true, false, false);
    this->conHexGraph_.addInnerCell(1, 2, 9, 11, 18, 19);
    this->conHexGraph_.addInnerCell(2, 3, 4, 11, 12, 13);
    this->conHexGraph_.addInnerCell(4, 5, 6, 13, 14, 15);
    this->conHexGraph_.addInnerCell(6, 7, 15, 17, 25, 26);
    this->conHexGraph_.addInnerCell(25, 26, 34, 35, 43, 44);
    this->conHexGraph_.addInnerCell(43, 44, 52, 53, 61, 62);
    this->conHexGraph_.addInnerCell(61, 62, 69, 71, 78, 79);
    this->conHexGraph_.addInnerCell(67, 68, 69, 76, 77, 78);
    this->conHexGraph_.addInnerCell(65, 66, 67, 74, 75, 76);
    this->conHexGraph_.addInnerCell(54, 55, 63, 65, 73, 74);
    this->conHexGraph_.addInnerCell(36, 37, 45, 46, 54, 55);
    this->conHexGraph_.addInnerCell(18, 19, 27, 28, 36, 37);
    this->conHexGraph_.addInnerCell(11, 12, 19, 21, 28, 29);
    this->conHexGraph_.addInnerCell(12, 13, 14, 21, 22, 23);
    this->conHexGraph_.addInnerCell(14, 15, 23, 25, 33, 34);
    this->conHexGraph_.addInnerCell(33, 34, 42, 43, 51, 52);
    this->conHexGraph_.addInnerCell(51, 52, 59, 61, 68, 69);
    this->conHexGraph_.addInnerCell(57, 58, 59, 66, 67, 68);
    this->conHexGraph_.addInnerCell(46, 47, 55, 57, 65, 66);
    this->conHexGraph_.addInnerCell(28, 29, 37, 38, 46, 47);
    this->conHexGraph_.addInnerCell(21, 22, 29, 31, 38, 39);
    this->conHexGraph_.addInnerCell(22, 23, 31, 33, 41, 42);
    this->conHexGraph_.addInnerCell(41, 42, 49, 51, 58, 59);
    this->conHexGraph_.addInnerCell(38, 39, 47, 49, 57, 58);
    this->conHexGraph_.addCenterCell(31, 39, 40, 41, 49);
    this->conHexGraph_.selfEdgeConnect();

    reset();
}
void ConHexEnv::reset()
{
    // reset board
    winner_ = Player::kPlayerNone;
    turn_ = Player::kPlayer1;

    actions_.clear();

    conHexGraph_.reset();

    board_.resize(kConHexBoardSize * kConHexBoardSize);
    fill(board_.begin(), board_.end(), Player::kPlayerNone);
}

bool ConHexEnv::act(const ConHexAction& action)
{
    if (!isLegalAction(action)) { return false; }

    int action_id = action.getActionID();

    if (config::env_conhex_use_swap_rule) {
        // Check if it's the second move and the chosen action is same as the first action
        if (actions_.size() == 1 && action_id == actions_[0].getActionID()) {
            // Player 2 has chosen to swap

            // Reflect the first move's position over the other diagonal
            int original_row = actions_[0].getActionID() / board_size_;
            int original_col = actions_[0].getActionID() % board_size_;

            int reflected_row = board_size_ - 1 - original_col;
            int reflected_col = board_size_ - 1 - original_row;
            int reflected_id = reflected_row * board_size_ + reflected_col;

            // Clear original move
            board_[actions_[0].getActionID()] = Player::kPlayerNone;
            conHexGraph_.reset();

            action_id = reflected_id;
        }
    }

    board_[action_id] = action.getPlayer();

    actions_.push_back(action);
    winner_ = updateWinner(action_id, action.getPlayer());
    turn_ = action.nextPlayer();
    return true;
}
Player ConHexEnv::updateWinner(int action_id, Player player)
{
    return this->conHexGraph_.cellCapture(action_id, player);
}

bool ConHexEnv::act(const std::vector<std::string>& action_string_args)
{
    return act(ConHexAction(action_string_args));
}

std::vector<ConHexAction> ConHexEnv::getLegalActions() const
{
    std::vector<ConHexAction> actions;
    for (int pos = 0; pos < board_size_ * board_size_; ++pos) {
        ConHexAction action(pos, turn_);
        if (!isLegalAction(action)) { continue; }
        actions.push_back(action);
    }
    return actions;
}

bool ConHexEnv::isPlaceable(int table_id) const
{
    /* action id 0-80
    ooooooooo
    oxoooooxo 10 16
    ooxoooxoo 20 24
    oooxoxooo 30 32
    ooooooooo
    oooxoxooo 48 50
    ooxoooxoo 56 60
    oxoooooxo 64 70
    ooooooooo
    */
    if (table_id == 10) return false;
    if (table_id == 16) return false;
    if (table_id == 20) return false;
    if (table_id == 24) return false;
    if (table_id == 30) return false;
    if (table_id == 32) return false;
    if (table_id == 48) return false;
    if (table_id == 50) return false;
    if (table_id == 56) return false;
    if (table_id == 60) return false;
    if (table_id == 64) return false;
    if (table_id == 70) return false;
    return true;
}

bool ConHexEnv::isLegalAction(const ConHexAction& action) const
{
    int action_id = action.getActionID();
    Player player = action.getPlayer();
    if (!(action_id >= 0 && action_id < board_size_ * board_size_)) return false;
    // assert(action_id >= 0 && action_id < board_size_ * board_size_);
    assert(player == Player::kPlayer1 || player == Player::kPlayer2);

    if (player != turn_) return false; // not player's turn

    if (config::env_conhex_use_swap_rule && actions_.size() == 1) {
        // swap rule
        return isPlaceable(action_id);
    }
    if (board_[action_id] == Player::kPlayerNone) {
        // non-swap rule
        // spot not be placed
        return isPlaceable(action_id);
    }
    return false; // illegal move
}

bool ConHexEnv::isTerminal() const
{
    return winner_ != Player::kPlayerNone;
}

float ConHexEnv::getEvalScore(bool is_resign /*= false*/) const
{
    if (is_resign) {
        return turn_ == Player::kPlayer1 ? -1.0f : 1.0f;
    }
    switch (winner_) {
        case Player::kPlayer1: return 1.0f;
        case Player::kPlayer2: return -1.0f;
        default: return 0.0f;
    }
}

std::vector<float> ConHexEnv::getFeatures(utils::Rotation rotation /*= utils::Rotation::kRotationNone*/) const
{
    /* 6 channels:
        0~1. own/opponent position
        2~3. own/opponent cell position
        4. Player1 turn
        5. Player2 turn
    */
    std::vector<float> vFeatures;
    for (int channel = 0; channel < 6; ++channel) {
        for (int pos = 0; pos < board_size_ * board_size_; ++pos) {
            int rotation_pos = pos;
            if (channel == 0) {
                vFeatures.push_back((board_[rotation_pos] == turn_ ? 1.0f : 0.0f));
            } else if (channel == 1) {
                vFeatures.push_back((board_[rotation_pos] == getNextPlayer(turn_, kConHexNumPlayer) ? 1.0f : 0.0f));
            } else if (channel == 2) {
                vFeatures.push_back((conHexGraph_.cellIsCapturedByPlayer(pos, turn_)) ? 1.0f : 0.0f);
            } else if (channel == 3) {
                vFeatures.push_back((conHexGraph_.cellIsCapturedByPlayer(pos, getNextPlayer(turn_, kConHexNumPlayer))) ? 1.0f : 0.0f);
            } else if (channel == 4) {
                vFeatures.push_back((turn_ == Player::kPlayer1 ? 1.0f : 0.0f));
            } else if (channel == 5) {
                vFeatures.push_back((turn_ == Player::kPlayer2 ? 1.0f : 0.0f));
            } 
        }
    }
    return vFeatures;
}

std::vector<float> ConHexEnv::getActionFeatures(const ConHexAction& action, utils::Rotation rotation /*= utils::Rotation::kRotationNone*/) const
{
    std::vector<float> action_features(board_size_ * board_size_, 0.0f);
    action_features[action.getActionID()] = 1.0f;
    return action_features;
}

std::string ConHexEnv::toString() const
{
    return conHexGraph_.toString();
}

std::vector<float> ConHexEnvLoader::getActionFeatures(const int pos, utils::Rotation rotation /* = utils::Rotation::kRotationNone */) const
{
    const ConHexAction& action = action_pairs_[pos].first;
    std::vector<float> action_features(getBoardSize() * getBoardSize(), 0.0f);
    int action_id = ((pos < static_cast<int>(action_pairs_.size())) ? action.getActionID() : utils::Random::randInt() % action_features.size());
    action_features[action_id] = 1.0f;
    return action_features;
}

} // namespace minizero::env::conhex
