#ifndef COMP3821_PROJ_HELPER_HEADER
#define COMP3821_PROJ_HELPER_HEADER

#include <vector>
#include <string>
#include <chess.hpp>
#include <unordered_set>
#include <set>

namespace helper {
    // Utility function for printing boards to terminal, primarily was used during development for debugging
    auto print_FEN_as_ASCII_board(std::string const& input) -> void;

    // Utility function used to convert a board from chess-library to a FEN string (according to our
    // requirements, which sets default values for non-positional/turn related information)
    auto board_to_FEN_wrapper(chess::Board const& board) -> std::string;

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

    // Finds the depth to mate for the state. If the state is not in the tablebase, -1 is returned
    auto get_depth_to_mate_for_state(std::string const& FEN_string, std::vector<std::unordered_set<std::string>> const& states_with_forceable_wins_for_white) -> int;

    // This function generates an endgame tablebase for the provided parameters,
    // done so according to the definition provided by our algorithm
    auto definitive_generate_tablebase(
        int const depth_to_mate_checked,
        int const max_pieces_present,
        std::vector<char> const starting_pieces
    ) -> std::vector<std::unordered_set<std::string>>;

    // This function probes an endgame tablebase to find an optimal move for a given board state,
    // done so according to the definition provided by our algorithm
    auto definitive_get_next_move(
        std::string const& FEN_string,
        std::vector<std::unordered_set<std::string>> const& depth_to_mate_forced_wins_for_white
    ) -> std::set<std::string>;
}


#endif // COMP3821_PROJ_HELPER_HEADER
