#include "./helper.h"
#include <catch.hpp>
#include <chess.hpp>
#include <set>

// Primarily will focus on end-to-end tests, as our project has the goal of determining
// the next optimal move to take (which is a functional requirement as opposed to one
// that can be decomposed as easily)





// These tests may take a few minutes to run due to the significant number of board states generated
// during checkmate generation for the tablebase
TEST_CASE("Three piece endgames with kKQ") {
    auto const tablebase = helper::definitive_generate_tablebase(10, 3, std::vector<char>{{'k', 'K', 'Q'}});

    SECTION("Mate in 1, no captures") {
        auto const FEN_string = "4k3/Q7/5K2/8/8/8/8/8 w - - 0 1";

        auto const expected_result = std::set<std::string>{{
            std::string{"4k3/4Q3/5K2/8/8/8/8/8 b - - 0 1"}
        }};

        auto const result = helper::definitive_get_next_move(FEN_string, tablebase);

        CHECK(result == expected_result);
    }

    SECTION("Mate in 9, no captures") {
        auto const FEN_string = "8/4k3/8/3Q4/8/5K2/8/8 w - - 0 1";

        auto const expected_result = std::set<std::string>{{
            std::string{"8/4k3/8/3Q4/6K1/8/8/8 b - - 0 1"},
            std::string{"8/4k3/8/3Q4/5K2/8/8/8 b - - 0 1"}
        }};

        auto const result = helper::definitive_get_next_move(FEN_string, tablebase);

        CHECK(result == expected_result);
    }
}

TEST_CASE("Other three piece endgames") {
    auto const tablebase = helper::definitive_generate_tablebase(10, 3, std::vector<char>{});

    SECTION("Mate in 5 with kKR") {
        auto const FEN_string = "5k2/8/8/3R1K2/8/8/8/8 w - - 0 1";

        auto const expected_result = std::set<std::string>{{
            std::string{"5k2/8/5K2/3R4/8/8/8/8 b - - 0 1"}
        }};

        auto const result = helper::definitive_get_next_move(FEN_string, tablebase);

        CHECK(result == expected_result);
    }

    SECTION("No forced wins for kKN") {
        auto const FEN_string = "5k2/8/5K2/2N5/8/8/8/8 b - - 0 1";

        auto const expected_result = std::set<std::string>{};

        auto const result = helper::definitive_get_next_move(FEN_string, tablebase);

        CHECK(result == expected_result);
    }

    SECTION("No forced wins for kKB") {
        auto const FEN_string = "7k/8/8/1B6/8/4K3/8/8 w - - 0 1";

        auto const expected_result = std::set<std::string>{};

        auto const result = helper::definitive_get_next_move(FEN_string, tablebase);

        CHECK(result == expected_result);
    }
}


struct Fixture {
    Fixture() {
        // A fixture is required to reuse/memoise the result from this tablebase generation
        tablebase = helper::definitive_generate_tablebase(5, 4, std::vector<char>{{'k', 'K', 'Q', 'n'}});
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

        auto const result = helper::definitive_get_next_move(FEN_string, tablebase);

        CHECK(result == expected_result);
    }

    SECTION("Mate in 5 with kKQn by zeroing (capturing the enemy knight)") {
        auto const FEN_string = "8/8/2Q2n1k/5K2/8/8/8/8 w - - 0 1";

        auto const expected_result = std::set<std::string>{{
            std::string{"8/8/2Q2K1k/8/8/8/8/8 b - - 0 1"}
        }};

        auto const result = helper::definitive_get_next_move(FEN_string, tablebase);

        CHECK(result == expected_result);
    }
}
