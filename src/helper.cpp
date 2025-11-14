#ifndef COMP3821_PROJ_HELPER
#define COMP3821_PROJ_HELPER


#include <vector>
#include <set>
#include <algorithm>
#include <string>

namespace helper {
    // We use FEN strings to represent the chess board states as we input them into chess-library.
    // We treat our player as the White player (since it doesn't matter which colour they are, only
    // whether it is their turn or not) and the opposite for the opponent.
    // We assume a board side length of 8, making the current code rather brittle to solving chess
    // games with larger boards.

    // private functions
    namespace {
        // Pieces are represented as characters in this representation
        auto const piece_types_without_kings = std::set<char>{'B', 'N', 'P', 'Q', 'R', 'b', 'n', 'p', 'q', 'r'};
        auto const num_board_squares_minus_one = 63;

        auto find_iter_piece(char const& prev) -> std::set<char>::iterator {
            return prev == 'K' ? helper::piece_types_without_kings.begin() : helper::piece_types_without_kings.find(prev);
        }
    }

    // To generate the FEN strings, we first need to choose which pieces to include in the strings,
    // so we return a vector of vectors, where the nested vectors represent the set of pieces we use
    auto generate_piece_combinations(int const num_pieces) -> std::vector<std::vector<char>> {
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

    // private functions
    namespace {
        // this performs the increment on the input vector to not need to copy it every time,
        // bad practice but its a bit faster
        auto increment_enumerator(std::vector<uint8_t>& input) -> void {
            auto iter = input.rbegin();
            while ((++(*iter)) > num_board_squares_minus_one) {
                *iter = 0;
                ++iter;
            }
        }

        auto has_enumerator_overlaps(std::vector<uint8_t> input) -> bool {
            std::sort(input.begin(), input.end());

            auto i = input.begin();
            auto j = ++(input.begin());

            while (j != input.end()) {
                if (*i == *j) {
                    return true;
                }

                ++i;
                ++j;
            }
            return false;
        }

        auto convert_array_to_FEN(std::array<char, 1 + num_board_squares_minus_one>& board) -> std::string {
            auto FEN_string = std::string{};
            // variable names of row for rank, col for file

            // we loop through each row of the chessboard to construct the FEN notation
            for (auto row_offset = 0; row_offset < 8; row_offset++) {
                auto empty_squares_in_a_row_in_rank = 0;

                // we loop through each square of the row to copy over the pieces
                for (auto col_offset = 0; col_offset < 8; col_offset++) {
                    auto const& currChar = board[(row_offset * 8) + col_offset];
                    if (currChar != '\0') {
                        // we add a number representing the gap of empty squares
                        if (empty_squares_in_a_row_in_rank) {
                            FEN_string.append(std::to_string(empty_squares_in_a_row_in_rank));
                            empty_squares_in_a_row_in_rank = 0;
                        }
                        // we add the current piece
                        FEN_string.push_back(currChar);
                    } else {
                        // if this is an empty row, we increment our current count
                        ++empty_squares_in_a_row_in_rank;
                    }
                }

                if (empty_squares_in_a_row_in_rank) {
                    FEN_string.append(std::to_string(empty_squares_in_a_row_in_rank));
                }

                if (row_offset != 7) {
                    FEN_string.push_back('/');
                }
            }

            // currently ignoring castling and en passants...
            FEN_string.append(" b - - 0 1");

            return FEN_string;
        }
    }

    // Now, for a given set of pieces we want to generate all possible board states
    // We filter our boards for checkmates for white, and remove any illegal/underfilled states
    auto generate_checkmates_for_piece_set_for_player(std::vector<char> const& pieces) -> void {
        // this lets us enumerate over the permutations of positions for each piece,
        // the uint8s represent the position of the piece on the board, and the index in our
        // enumerator correlates to its respective piece in the provided input vector of pieces
        auto enumerator = std::vector<uint8_t>(pieces.size(), 0);
        auto const max_enumerator_value = std::vector<uint8_t>(pieces.size(), num_board_squares_minus_one);

        // we can ignore the first and last cases for the enumerator because they are guaranteed to
        // have overlaps, and thus are underfilled
        while (enumerator != max_enumerator_value) {
            increment_enumerator(enumerator);
            if (has_enumerator_overlaps(enumerator)) {
                continue;
            }

            auto board_state = std::array<char, 1 + num_board_squares_minus_one>{};
            for (auto i = 0; i < pieces.size(); ++i) {
                board_state[enumerator[i]] = pieces[i];
            }

            auto FEN_string = convert_array_to_FEN(board_state);
            std::cout << FEN_string << "\n";
        }
    }
}


#endif // COMP3821_PROJ_HELPER
