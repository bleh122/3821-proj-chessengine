#ifndef COMP3821_PROJ_HELPER_HEADER
#define COMP3821_PROJ_HELPER_HEADER

#include <vector>
#include <string>
#include <unordered_set>

namespace helper {
    // Utility function for printing boards to terminal, primarily was used during development for debugging
    auto print_FEN_as_ASCII_board(std::string const& input) -> void;

    // Generates a list of combinations of pieces, where they are legal (contain kK) and are all
    // less than or equal in size to our input integer
    auto generate_piece_combinations(int const max_num_pieces) -> std::vector<std::vector<char>>;

    // Generates the power set of the given set of pieces
    auto generate_subsets_of_piece_combination(std::vector<char> const& max_piece_combination) -> std::vector<std::vector<char>>;


    // For a given set of pieces we want to generate all possible board states
    // We filter our boards for checkmates for white, and remove any illegal/underfilled states
    auto generate_checkmates_for_piece_set_for_player(std::vector<char> const& pieces) -> std::vector<std::string>;

    // Generates the direct predecessor board states for our current state, i.e. states where
    // a player takes one move to result in the current state (player turn matters)
    auto generate_predecessor_board_states(std::string const& FEN_string, bool const isWhiteTurn, int const max_pieces_present) -> std::unordered_set<std::string>;

    // Generates all successor board states to our input state (reached from taking a legal move)
    auto generate_successor_boards(std::string const& curr_FEN) -> std::unordered_set<std::string>;


    // If every successor board to our current board is already known as a forced win, then this is
    // a state where we can force a win (a state can go from returning false to returning true
    // upon repeated queries as our set of known forced wins increases)
    auto is_forced_win(std::string const& current_board, std::unordered_set<std::string> const& known_forced_wins) -> bool;
}


#endif // COMP3821_PROJ_HELPER_HEADER
