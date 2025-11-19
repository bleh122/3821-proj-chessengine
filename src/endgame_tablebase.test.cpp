#include "./helper.h"
#include <catch.hpp>
#include <chess.hpp>
#include <set>

// Primarily will focus on end-to-end tests, as our project has the goal of determining
// the next optimal move to take (which is a functional requirement as opposed to one
// that can be decomposed as easily)

// This helper is a modification of the code from the ./run_engine program, generating the tablebase
auto test_helper_generate_tablebase(
    int const depth_to_mate_checked,
    int const max_pieces_present,
    std::vector<char> const starting_pieces
) {
    // Generate combinations of pieces from which to generate checkmates for retrograde analysis
    auto piece_combinations = std::vector<std::vector<char>>{};
    if (starting_pieces.empty()) {
        piece_combinations = std::move(helper::generate_piece_combinations(max_pieces_present));
    } else {
        piece_combinations = std::move(helper::generate_subsets_of_piece_combination(starting_pieces));
    }

    auto checkmate_states = std::unordered_set<std::string>();
    for (auto i : piece_combinations) {
        auto combination = std::string{};
        for (auto j : i) {
            combination.push_back(j);
        }

        auto some_checkmates = helper::generate_checkmates_for_piece_set_for_player(i);
        for (auto j : some_checkmates) {
            checkmate_states.insert(j);
        }
    }

    auto states_with_forceable_wins_for_white = std::unordered_set<std::string>();
    states_with_forceable_wins_for_white.insert(checkmate_states.begin(), checkmate_states.end());

    auto depth_to_mate_forced_wins_for_white = std::vector<std::unordered_set<std::string>>{};
    depth_to_mate_forced_wins_for_white.emplace_back(checkmate_states);


    while (depth_to_mate_forced_wins_for_white.size() <= depth_to_mate_checked) {
        auto curr_depth_forced_wins = std::unordered_set<std::string>();
        for (auto i : depth_to_mate_forced_wins_for_white.back()) {
            if (depth_to_mate_forced_wins_for_white.size() % 2 == 1) {
                auto possible_predecessor_boards = helper::generate_predecessor_board_states(i, true, max_pieces_present);

                for (auto j : possible_predecessor_boards) {
                    if (not states_with_forceable_wins_for_white.contains(j)) {
                        curr_depth_forced_wins.emplace(j);
                    }
                }
            } else {
                auto possible_predecessor_boards = helper::generate_predecessor_board_states(i, false, max_pieces_present);
                for (auto j: possible_predecessor_boards) {
                    if (helper::is_forced_win(j, states_with_forceable_wins_for_white)) {
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

    return depth_to_mate_forced_wins_for_white;
}

// This helper is a modified version of the program ./get_next_move, findding the set of optimal moves
// (where optimal means it takes the fewest moves to force checkmate)
auto test_helper_get_next_move(
    std::string const& FEN_string,
    std::vector<std::unordered_set<std::string>> const& depth_to_mate_forced_wins_for_white
) {
    auto res = std::set<std::string>{};
    auto const depth_to_mate = helper::get_depth_to_mate_for_state(FEN_string, depth_to_mate_forced_wins_for_white);

    if (depth_to_mate == -1) return res;

    auto board = chess::Board(FEN_string);

    auto movelist = chess::Movelist();
    chess::movegen::legalmoves(movelist, board);

    for (auto curr_move : movelist) {
        auto successor_board = chess::Board(FEN_string);
        successor_board.makeMove(curr_move);
        auto successor_FEN = helper::board_to_FEN_wrapper(successor_board);

        if (depth_to_mate_forced_wins_for_white[depth_to_mate - 1].contains(successor_FEN)) {
            res.emplace(successor_FEN);
        }
    }

    return res;
}


// These tests may take a few minutes to run due to the significant number of board states generated
// during checkmate generation for the tablebase
TEST_CASE("Three piece endgames with kKQ") {
    auto const tablebase = test_helper_generate_tablebase(10, 3, std::vector<char>{{'k', 'K', 'Q'}});

    SECTION("Mate in 1, no captures") {
        auto const FEN_string = "4k3/Q7/5K2/8/8/8/8/8 w - - 0 1";

        auto const expected_result = std::set<std::string>{{
            std::string{"4k3/4Q3/5K2/8/8/8/8/8 b - - 0 1"}
        }};

        auto const result = test_helper_get_next_move(FEN_string, tablebase);

        CHECK(result == expected_result);
    }

    SECTION("Mate in 9, no captures") {
        auto const FEN_string = "8/4k3/8/3Q4/8/5K2/8/8 w - - 0 1";

        auto const expected_result = std::set<std::string>{{
            std::string{"8/4k3/8/3Q4/6K1/8/8/8 b - - 0 1"},
            std::string{"8/4k3/8/3Q4/5K2/8/8/8 b - - 0 1"}
        }};

        auto const result = test_helper_get_next_move(FEN_string, tablebase);

        CHECK(result == expected_result);
    }
}

TEST_CASE("Other three piece endgames") {
    auto const tablebase = test_helper_generate_tablebase(10, 3, std::vector<char>{});

    SECTION("Mate in 5 with kKR") {
        auto const FEN_string = "5k2/8/8/3R1K2/8/8/8/8 w - - 0 1";

        auto const expected_result = std::set<std::string>{{
            std::string{"5k2/8/5K2/3R4/8/8/8/8 b - - 0 1"}
        }};

        auto const result = test_helper_get_next_move(FEN_string, tablebase);

        CHECK(result == expected_result);
    }

    SECTION("No forced wins for kKN") {
        auto const FEN_string = "5k2/8/5K2/2N5/8/8/8/8 b - - 0 1";

        auto const expected_result = std::set<std::string>{};

        auto const result = test_helper_get_next_move(FEN_string, tablebase);

        CHECK(result == expected_result);
    }

    SECTION("No forced wins for kKB") {
        auto const FEN_string = "7k/8/8/1B6/8/4K3/8/8 w - - 0 1";

        auto const expected_result = std::set<std::string>{};

        auto const result = test_helper_get_next_move(FEN_string, tablebase);

        CHECK(result == expected_result);
    }
}


struct Fixture {
    Fixture() {
        // A fixture is required to reuse/memoise the result from this tablebase generation
        tablebase = test_helper_generate_tablebase(5, 4, std::vector<char>{{'k', 'K', 'Q', 'n'}});
    }

    std::vector<std::unordered_set<std::string>> tablebase;
};


TEST_CASE_METHOD(Fixture, "Four piece endgame for kKQn") {
    SECTION("Mate in 2 with kKQn without capture") {
        auto const FEN_string = "6k1/8/5K2/8/1n6/7Q/8/8 w - - 0 1";

        auto const expected_result = std::set<std::string>{{
            std::string{"6k1/8/5K2/8/1n6/8/6Q1/8 b - - 0 1"},
            std::string{"6k1/8/5K2/8/1n6/6Q1/8/8 b - - 0 1"},
            std::string{"6k1/8/5K2/8/1n4Q1/8/8/8 b - - 0 1"}
        }};

        auto const result = test_helper_get_next_move(FEN_string, tablebase);

        CHECK(result == expected_result);
    }

    SECTION("Mate in 5 with kKQn by zeroing (capturing the enemy knight)") {
        auto const FEN_string = "8/8/2Q2n1k/5K2/8/8/8/8 w - - 0 1";

        auto const expected_result = std::set<std::string>{{
            std::string{"8/8/2Q2K1k/8/8/8/8/8 b - - 0 1"}
        }};

        auto const result = test_helper_get_next_move(FEN_string, tablebase);

        CHECK(result == expected_result);
    }
}
