#include <iostream>
#include <stdio.h>
#include <chess.hpp>
#include <stdint.h>
#include <array>
#include "./helper.h"
#include <string>
#include <unordered_set>
#include <fstream>

// less than two pieces is illegal, more than 5 is too expensive
auto constexpr MIN_PIECES_ALLOWED = 2;
auto constexpr MAX_PIECES_ALLOWED = 5;

// This program will generate an output csv file to be used as a tablebase for the get_next_move file
int main(int argc, char** argv) {
    // Processing command line arguments
    if (argc < 3) {
        std::cout << "Usage is:\n"
            << "./run_engine     <int>max_depth_to_mate   <int>max_num_pieces    <optional string>starting_pieces\n\n\n"

            << "\tmax_depth_to_mate is an integer that tells our engine how many unmoves from "
            << "checkmate our engine should explore.\n\n"

            << "\tmax_num_pieces is an integer that tells our engine how many pieces we wish to "
            << "solve the game for (i.e. board states with pieces less than or equal to the amount "
            << "provided will be checked for).\n\n"

            << "\tstarting_pieces is an optional string parameter with no spaces containing "
            << "letters from {K, Q, R, B, N, k, q, r, b, n}, e.g. the string 'KkQn' will suffice, "
            << "using FEN notation for the pieces, with capital letters representing pieces of the "
            << "white player (which we assume to be our user to avoid duplication of logic for "
            << "handling otherwise).\n\tThis represents the set of pieces which we are solving "
            << "various board states for. In the case where this parameter is empty, we solve for "
            << "all combinations of starting pieces that fit the earlier constraints (this "
            << "will likely take significantly longer due to its combinatoric nature).\n\n"
            ;

        return 0;
    }

    const int depth_to_mate_checked = std::stoi(argv[1]);
    const int max_pieces_present = std::stoi(argv[2]);
    const auto starting_pieces_string = (argc == 4) ? std::string{argv[3]} : std::string{};
    const auto starting_pieces = std::vector<char>{starting_pieces_string.begin(), starting_pieces_string.end()};

    if (
        max_pieces_present < MIN_PIECES_ALLOWED or
        max_pieces_present > MAX_PIECES_ALLOWED or
        ((starting_pieces.size() != 0) and (max_pieces_present != starting_pieces.size()))
    ) {
        std::cout << "Error: Bad max piece count, please be at least 2 and less than 5, "
            << "and be equal to the number of pieces provided in the starting pieces string.\n"
            ;
        return 1;
    }


    // ALGORITHM IMPLEMENTATION FOR ENDGAME TABLEBASE GENERATION BEGINS HERE
    // Generate combinations of pieces from which to generate checkmates for retrograde analysis
    auto piece_combinations = std::vector<std::vector<char>>{};
    if (starting_pieces.empty()) {
        // This version of the function generates all piece combinations with a number of pieces
        // less than or equal to the max_pieces_present supplied
        piece_combinations = std::move(helper::generate_piece_combinations(max_pieces_present));
    } else {
        // This version of the function generates all piece combinations which are a subset of our
        // given piece combination
        piece_combinations = std::move(helper::generate_subsets_of_piece_combination(starting_pieces));
    }

    // This is according to n + k - 1 choose k, where n = 10, k = max_pieces_present, unless pieces are provided
    std::cout << "There are " << piece_combinations.size() << " combinations of pieces.\n";

    // For now we use an unordered set of strings, as the chess::Board type does not overload the
    // == operator in a manner that allows for unordered_set to be used for it
    auto checkmate_states = std::unordered_set<std::string>();
    for (auto i : piece_combinations) {
        auto combination = std::string{};
        for (auto j : i) {
            combination.push_back(j);
        }

        std::cout << "Now generating checkmates for new piece combination: " << combination << "\n";
        auto some_checkmates = helper::generate_checkmates_for_piece_set_for_player(i);
        for (auto j : some_checkmates) {
            checkmate_states.insert(j);
        }
    }


    // A set of all states with forceable wins for white (regardless of depth to mate, and 
    // containing board states from the turns of both white and black players)
    auto states_with_forceable_wins_for_white = std::unordered_set<std::string>();
    states_with_forceable_wins_for_white.insert(checkmate_states.begin(), checkmate_states.end());

    // We use a vector to allow us to store the following information:
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
        std::cout << "Checking for new move depth: "
            << depth_to_mate_forced_wins_for_white.size()
            << ", last iteration had "
            << depth_to_mate_forced_wins_for_white.back().size()
            << " boards.\n";
        auto curr_depth_forced_wins = std::unordered_set<std::string>();
        for (auto i : depth_to_mate_forced_wins_for_white.back()) {
            if (depth_to_mate_forced_wins_for_white.size() % 2 == 1) {
                // It's white's move this turn
                // These are states where white can select a move that will result in them winning
                auto possible_predecessor_boards = helper::generate_predecessor_board_states(i, true, max_pieces_present);

                for (auto j : possible_predecessor_boards) {
                    // Avoid recalculation for states we already know to be winning
                    if (not states_with_forceable_wins_for_white.contains(j)) {
                        curr_depth_forced_wins.emplace(j);
                    }
                }
            } else {
                // It's black's move this turn
                // These are states where whatever move black takes, they will lose in the end
                auto possible_predecessor_boards = helper::generate_predecessor_board_states(i, false, max_pieces_present);
                for (auto j: possible_predecessor_boards) {
                    if (helper::is_forced_win(j, states_with_forceable_wins_for_white)) {
                        // Avoid recalculation for states we already know to be winning
                        if (not states_with_forceable_wins_for_white.contains(j)) {
                            curr_depth_forced_wins.emplace(j);
                        }
                    }
                }
            }
        }

        depth_to_mate_forced_wins_for_white.emplace_back(curr_depth_forced_wins);
        states_with_forceable_wins_for_white.insert(curr_depth_forced_wins.begin(), curr_depth_forced_wins.end());
    }


    // POST PROCESSING OF OUR RESULTANT ENDGAME TABLEBASE OCCURS HERE, MAINLY FOR SAVING OUTPUT
    auto output_file = std::ofstream("output.csv");

    // C++ streaming input from a file splits it by whitespace, so here we format our output
    // to take advantage of this in the form "depth_to_mate FEN_position_segment player_turn"
    // for each row of a CSV file
    for (auto i = 0; i < depth_to_mate_forced_wins_for_white.size(); ++i) {
        for (auto j : depth_to_mate_forced_wins_for_white[i]) {
            auto FEN_position_segment = std::string{};

            auto j_iter = j.begin();
            for (; *j_iter != ' '; ++j_iter) {
                FEN_position_segment.push_back(*j_iter);
            }

            auto player_turn = *(++j_iter);

            output_file << i << " " << FEN_position_segment << " " << player_turn << "\n";
        }
    }

    return 0;
}


