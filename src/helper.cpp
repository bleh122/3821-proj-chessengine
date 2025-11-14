#ifndef COMP3821_PROJ_HELPER
#define COMP3821_PROJ_HELPER


#include <vector>
#include <set>
#include <algorithm>

namespace helper {
    // We use FEN strings to represent the chess board states as we input them into chess-library.

    // Pieces are represented as characters in this representation
    auto const piece_types_without_kings = std::set<char>{'p', 'n', 'b', 'r', 'q', 'P', 'N', 'B', 'R', 'Q'};

    namespace {
        auto find_iter_piece(char& prev) -> std::set<char>::iterator {
            return prev == 'K' ? helper::piece_types_without_kings.begin() : helper::piece_types_without_kings.find(prev);
        }
    }

    // To generate the FEN strings, we first need to choose which pieces to include in the strings,
    // so we return a vector of vectors, where the nested vectors represent the set of pieces we use
    auto generate_piece_combinations(int num_pieces) -> std::vector<std::vector<char>> {
        auto states = std::vector<std::vector<char>>{};
        states.emplace_back(std::vector<char>{'k', 'K'});
        for (auto i = 2; i < num_pieces; ++i) {
            auto temp = std::vector<std::vector<char>>{};
            for (auto j = states.begin(); j != states.end(); ++j) {
                // since we imposed an order on the pieces, we can do a trick to now generate
                // combinations instead of permutations
                for (auto k = find_iter_piece(j->back()); k != piece_types_without_kings.cend(); ++k) {
                    auto newest_state = std::vector<char>{j->begin(), j->end()};
                    newest_state.emplace_back(*k);
                    temp.emplace_back(newest_state);
                }
            }
            states = std::move(temp);
        }

        return states;
    }
}


#endif // COMP3821_PROJ_HELPER
