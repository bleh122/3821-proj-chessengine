#include <iostream>
#include <stdio.h>
#include <chess.hpp>
#include <stdint.h>
#include <array>
#include "./helper.cpp"
#include <string>
#include <unordered_set>
#include <fstream>

// less than two pieces is illegal, more than 5 is too expensive
auto constexpr MIN_PIECES_ALLOWED = 2;
auto constexpr MAX_PIECES_ALLOWED = 5;

// This program will generate an output csv file to be used as a tablebase for the get_next_move file
int main(int argc, char** argv) {
    int num_ending_pieces;
    int max_pieces_present;
    int depth_to_mate_checked;
    auto starting_pieces = std::vector<char>{};

    // process/acquire arguments to tablebase generator
    if (argc < 5) {
        std::cout << "Enter the number of pieces that will be present at checkmate (integer): ";
        if (not (std::cin >> num_ending_pieces)) {
            std::cout << "Error: incorrect input type for integer.\n";
            return 1;
        }

        std::cout << "Enter the max number of pieces allowed to appear (integer): ";
        if (not (std::cin >> max_pieces_present)) {
            std::cout << "Error: incorrect input type for integer.\n";
            return 1;
        }

        std::cout << "Enter the max number of unmoves/depth to mate to check (integer): ";
        if (not (std::cin >> depth_to_mate_checked)) {
            std::cout << "Error: incorrect input type for integer.\n";
            return 1;
        }

        std::cout   << "Enter the pieces (ignoring position) currently on your board (as a string "
                    << "with no spaces, containing any of [K, Q, R, B, N, P, k, q, r, b, n, p], "
                    << "where capital letters are white pieces and lowercase are black pieces, "
                    << "i.e. FEN notation for pieces, e.g. kKQ). Enter the empty string to test "
                    << "for all possible pieces: ";
        std::string temp_string;
        if (not (std::cin >> temp_string)) {
            std::cout << "Error: string not entered.\n";
            return 1;
        }
        for (auto i : temp_string) {
            starting_pieces.emplace_back(i);
        }
    } else {
        num_ending_pieces = std::stoi(argv[1]);
        max_pieces_present = std::stoi(argv[2]);
        depth_to_mate_checked = std::stoi(argv[3]);

        for (auto i : std::string{argv[4]}) {
            starting_pieces.emplace_back(i);
        }
    }

    if (num_ending_pieces < MIN_PIECES_ALLOWED or num_ending_pieces > MAX_PIECES_ALLOWED) {
        std::cout << "Error: Bad end piece count, please be at least 2 and less than 5.\n";
        return 1;
    } else if (num_ending_pieces > max_pieces_present) {
        std::cout << "Error: Bad max piece count, is less than end piece count.\n";
        return 1;
    }

    // generate combinations of pieces from which to generate checkmates for retrograde analysis
    auto piece_combinations = std::vector<std::vector<char>>{};
    if (starting_pieces.empty()) {
        piece_combinations = helper::generate_piece_combinations(num_ending_pieces);
    } else {
        piece_combinations.emplace_back(starting_pieces);
    }

    // this is according to n + k - 1 choose k, where n = 10, k = num_ending_pieces, unless pieces are provided
    std::cout << "There are " << piece_combinations.size() << " combinations of pieces.\n";

    // for now we use an unordered set of strings, because afaik gcc has some error when trying to
    // use the overriden equality operator for chess::Board for comparisons in the unordered set
    // can maybe use tries in the future since they are strings?
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
            // break;
        }
    }

    // a set of all states with forceable wins for white (regardless of moves to mate)
    auto states_with_forceable_wins_for_white = std::unordered_set<std::string>();
    states_with_forceable_wins_for_white.insert(checkmate_states.begin(), checkmate_states.end());

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
        std::cout   << "Checking for new move depth: "
                    << depth_to_mate_forced_wins_for_white.size()
                    << ", last iteration had "
                    << depth_to_mate_forced_wins_for_white.back().size()
                    << " boards.\n";
        auto temp = std::unordered_set<std::string>();
        for (auto i : depth_to_mate_forced_wins_for_white.back()) {
            if (depth_to_mate_forced_wins_for_white.size() % 2 == 1) {
                // it's white's move this turn
                // these are states where white can select a move that will result in them winning
                auto possible_predecessor_boards = helper::generate_predecessor_board_states(i, true, max_pieces_present);

                for (auto j : possible_predecessor_boards) {
                    // avoid recalculation for states we already know to be winning
                    if (not states_with_forceable_wins_for_white.contains(j)) {
                        temp.emplace(j);
                    }
                }
            } else {
                // it's black's move this turn
                // these are states where whatever move black takes, they will lose in the end
                auto possible_predecessor_boards = helper::generate_predecessor_board_states(i, false, max_pieces_present);
                for (auto j: possible_predecessor_boards) {
                    if (helper::is_forced_win(j, states_with_forceable_wins_for_white)) {
                        if (not states_with_forceable_wins_for_white.contains(j)) {
                            temp.emplace(j);
                        }
                    }
                }
                // break;
            }
        }
        depth_to_mate_forced_wins_for_white.emplace_back(temp);
        states_with_forceable_wins_for_white.insert(temp.begin(), temp.end());
    }

    // output our results to a file storing our results
    auto output_file = std::ofstream("output.csv");

    // c++ streaming input from a file splits it by whitespace, so we can try format our output
    // to take advantage of this in the form "depth_to_mate FEN_position_segment player_turn"
    // for each row of the csv
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


