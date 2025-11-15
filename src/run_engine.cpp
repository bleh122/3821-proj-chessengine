#include <iostream>
#include <stdio.h>
#include <chess.hpp>
#include <stdint.h>
#include <array>
#include "./helper.cpp"
#include <string>
#include <unordered_set>

int main(int argc, char** argv) {
    int num_ending_pieces;
    int max_pieces_present;
    int depth_to_mate_checked;

    if (argc < 4) {
        std::cout << "Enter the number of pieces that will be present at checkmate: ";
        if (not (std::cin >> num_ending_pieces)) {
            std::cout << "Error: incorrect input type for integer\n";
            return 0;
        }

        std::cout << "Enter the max number of pieces allowed to appear: ";
        if (not (std::cin >> max_pieces_present)) {
            std::cout << "Error: incorrect input type for integer\n";
            return 0;
        }

        std::cout << "Enter the max number of unmoves/depth to mate to check: ";
        if (not (std::cin >> depth_to_mate_checked)) {
            std::cout << "Error: incorrect input type for integer\n";
            return 0;
        }
    } else {
        num_ending_pieces = std::stoi(argv[1]);
        max_pieces_present = std::stoi(argv[2]);
        depth_to_mate_checked = std::stoi(argv[3]);
    }

    if (num_ending_pieces < 3 or num_ending_pieces > 5) {
        std::cout << "Error: Bad end piece count, please be greater than 2 and less than 5\n";
        return 1;
    } else if (num_ending_pieces > max_pieces_present) {
        std::cout << "Error: Bad max piece count, is less than end piece count\n";
        return 1;
    }

    // for now we use an unordered set of strings, because afaik gcc has some error when trying to
    // use the overriden equality operator for chess::Board for comparisons in the unordered set
    // can maybe use tries in the future since they are strings?
    auto piece_combinations = helper::generate_piece_combinations(num_ending_pieces);
    // this is according to n + k - 1 choose k, where n = 10, k = num_ending_pieces
    std::cout << piece_combinations.size() << "\n";

    auto checkmate_states = std::unordered_set<std::string>();
    for (auto i : piece_combinations) {
        std::cout << "now generating checkmates for new piece combination\n";
        auto some_checkmates = helper::generate_checkmates_for_piece_set_for_player(i);
        for (auto j : some_checkmates) {
            checkmate_states.insert(j);
            // break;
        }
    }


    // a set of all states with forceable wins for white (regardless of moves to mate)
    auto all_white_player_states_with_forceable_wins = std::unordered_set<std::string>();
    all_white_player_states_with_forceable_wins.insert(checkmate_states.begin(), checkmate_states.end());

    // we use a vector to allow us to store the following:
    // let n = index of element in vector
    // if n is even, then the element represents it being the black players turn and there being n
    //      moves left before forced checkmate (i.e. the black player can take any move and will
    //      still lose)
    // if n is odd, then the element represents it being the white players turn and there being n
    //      moves left before forced checkmate (i.e. the white player has some move to take that
    //      will allow them to force a win from that point onwards)
    auto depth_to_mate_forced_wins_for_white = std::vector<std::unordered_set<std::string>>{};
    depth_to_mate_forced_wins_for_white.emplace_back(checkmate_states);


    while (depth_to_mate_forced_wins_for_white.size() <= depth_to_mate_checked) {
        std::cout << "Checking for new move depth, last iteration had this many boards: " << depth_to_mate_forced_wins_for_white.back().size() << "\n";
        auto temp = std::unordered_set<std::string>();
        for (auto i : depth_to_mate_forced_wins_for_white.back()) {
            if (depth_to_mate_forced_wins_for_white.size() % 2 == 1) {
                // it's white's move this turn
                // these are states where white can select a move that will result in them winning
                auto possible_predecessor_boards = helper::generate_predecessor_board_states(i, true);

                for (auto j : possible_predecessor_boards) {
                    // avoid recalculation for states we already know to be winning
                    if (not all_white_player_states_with_forceable_wins.contains(j)) {
                        temp.emplace(j);
                    }
                }
            } else {
                // it's black's move this turn
                // these are states where whatever move black takes, they will lose in the end
                auto possible_predecessor_boards = helper::generate_predecessor_board_states(i, false);
                for (auto j: possible_predecessor_boards) {
                    if (helper::is_forced_win(j, all_white_player_states_with_forceable_wins)) {
                        if (not all_white_player_states_with_forceable_wins.contains(j)) {
                            temp.emplace(j);
                        }
                    }
                }
                // break;
            }
        }
        depth_to_mate_forced_wins_for_white.emplace_back(temp);
        all_white_player_states_with_forceable_wins.insert(temp.begin(), temp.end());
    }

    auto counter = 0;
    for (auto i : depth_to_mate_forced_wins_for_white) {
        std::cout << "\n\n\nDEPTH TO MATE: " << counter << "\n";
        for (auto j : i) {
            std::cout << "FEN: " << j << "\n";
            // helper::print_FEN_as_ASCII_board(j);

        }
        counter++;
    }

    return 0;
}


