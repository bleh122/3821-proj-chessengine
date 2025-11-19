#ifndef COMP3821_PROJ_HELPER
#define COMP3821_PROJ_HELPER


#include <vector>
#include <set>
#include <algorithm>
#include <string>
#include <chess.hpp>
#include <unordered_set>
#include <numeric>
#include "helper.h"

namespace helper {
    // LIST OF ASSUMPTIONS USED IN OUR IMPLEMENTATION:
    // We use FEN strings to represent the chess board states as we input them into chess-library.
    // We treat our player as the White player (since it doesn't matter which colour they are, only
    // whether it is their turn or not) and the opposite for the opponent.
    // We assume a board side length of 8, making the current code rather brittle to solving chess
    // games with larger boards.
    // We currently assume that endgames do not include pawns due to computational constraints

    // Private functions and constants/magic numbers
    namespace {
        // Pieces are represented as characters in this representation, we ignore pawns currently
        auto const PIECE_TYPES_WITHOUT_KINGS = std::set<char>{'B', 'N', 'Q', 'R', 'b', 'n', 'q', 'r'};
        // auto const PIECE_TYPES_WITHOUT_KINGS = std::set<char>{'B', 'N', 'P', 'Q', 'R', 'b', 'n', 'p', 'q', 'r'};

        auto const NUM_BOARD_SQUARES = 64;

        auto find_iter_piece(char const& prev) -> std::set<char>::iterator {
            return prev == 'K' ? helper::PIECE_TYPES_WITHOUT_KINGS.begin() : helper::PIECE_TYPES_WITHOUT_KINGS.find(prev);
        }

        // This performs the increment on the input vector, avoiding time spent copying
        auto increment_enumerator(std::vector<uint8_t>& input) -> void {
            auto iter = input.rbegin();
            while ((++(*iter)) > NUM_BOARD_SQUARES - 1) {
                *iter = 0;
                ++iter;
            }
        }

        // Determines if input enumerator value, when used to create a board, will result in missing
        // pieces due to position overlap
        auto contains_enumerator_overlaps(std::vector<uint8_t> input) -> bool {
            std::sort(input.begin(), input.end());
            auto i = input.begin();
            auto j = ++(input.begin());

            while (j != input.end()) {
                if (*i == *j) return true;

                ++i;
                ++j;
            }
            return false;
        }

        // Converts our array representation to a FEN string
        auto convert_array_to_FEN(
            std::array<char, NUM_BOARD_SQUARES> const& board,
            bool const isWhiteTurn
        ) -> std::string {
            auto FEN_string = std::string{};

            // we loop through each row/rank of the chessboard to construct the FEN notation
            for (auto row_offset = 0; row_offset < 8; row_offset++) {
                auto empty_squares_in_a_row_in_rank = 0;

                // we loop through each square of the row/rank to copy over the pieces
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

            // Our current FEN string generation ignores castling and en passants
            FEN_string.append(isWhiteTurn ? " w - - 0 1" : " b - - 0 1");

            return FEN_string;
        }

        // Assuming that both players have kings
        auto is_legal_board_state(chess::Board& board) -> bool {
            // Check if king of player who just made move is in check (and thus if game is illegal)
            board.makeNullMove();
            if (board.inCheck()) {
                return false;
            }
            board.makeNullMove();
            return true;
        }

        // Assuming that board state is legal, and current turn is black
        auto is_checkmate_win_for_white(chess::Board& board) -> bool {
            return board.inCheck() and (board.isGameOver().first == chess::GameResultReason::CHECKMATE);
        }

        // Converts FEN string to array representation of pieces
        auto convert_FEN_to_array(std::string const& FEN) -> std::array<char, NUM_BOARD_SQUARES>  {
            auto result_array = std::array<char, NUM_BOARD_SQUARES>{};
            auto index = 0;

            for (auto curr = FEN.begin(); curr != FEN.end(); ++curr) {
                if (*curr == ' ') break;
                if (*curr == '/') continue;

                if (std::isdigit(*curr)) {
                    int empty_squares = (*curr) - '0';
                    index += empty_squares;
                } else {
                    result_array[index] = *curr;
                    ++index;
                }
            }

            return result_array;
        }

        // Helper function used during debugging
        auto print_board_array_representation(std::array<char, NUM_BOARD_SQUARES> const& board) {
            for (auto i = 0; i < 64; i++) {
                if (i % 8 == 0 and i != 0) {
                    std::cout << "\n";
                }
                if (board[i] == '\0') {
                    std::cout << "_";
                } else {
                    std::cout << board[i];
                }

            }
            std::cout << "\n\n";
        }

        // Chess notation has different order of counting for rank (row), so to convert it to our
        // contiguous array indices we have to translate it like so:
        auto convert_square_to_index_for_array(chess::Square const& sq) -> int {
            int const row_offset = 7 - sq.rank();
            int const col_offset = sq.file();

            return (row_offset * 8) + col_offset;
        }

        // Inverse of above
        auto convert_array_index_to_square(int index) -> chess::Square {
            int rank = 7 - (index / 8);
            int file = index % 8;
            return chess::Square(chess::Rank(rank), chess::File(file));
        }

        auto get_piece_possible_predecessor_locations(
            chess::Piece const& piece,
            chess::Square const& current_position,
            chess::Bitboard const& current_occupied_spaces
        ) -> chess::Bitboard {
            switch (piece.internal()) {
                ///TODO: and logic for pawn unmoving and unpromotion
                // currently this is handled in a separate function due to the nature of pawn uncaptures
                case chess::Piece::WHITEPAWN:
                case chess::Piece::BLACKPAWN:
                    return chess::Bitboard();
                break;

                case chess::Piece::WHITEQUEEN:
                case chess::Piece::BLACKQUEEN:
                    return chess::attacks::queen(current_position, current_occupied_spaces);
                break;

                case chess::Piece::WHITEROOK:
                case chess::Piece::BLACKROOK:
                    return chess::attacks::rook(current_position, current_occupied_spaces);
                break;

                case chess::Piece::WHITEBISHOP:
                case chess::Piece::BLACKBISHOP:
                    return chess::attacks::bishop(current_position, current_occupied_spaces);
                break;

                case chess::Piece::WHITEKNIGHT:
                case chess::Piece::BLACKKNIGHT:
                    return chess::attacks::knight(current_position);
                break;

                case chess::Piece::WHITEKING:
                case chess::Piece::BLACKKING:
                    return chess::attacks::king(current_position);
                break;

                default:
                    std::cout << "Error, piece type is impossible?\n";
                    std::abort();
                break;
            }
        }

        auto piece_type_belongs_to_player(char piece_type, bool isWhiteSide) -> bool {
            return (static_cast<bool>(isupper(piece_type)) == isWhiteSide);
        }

        auto perform_unmove_or_uncapture(
            std::array<char, NUM_BOARD_SQUARES> const& board_array_representation,
            bool const isWhiteTurn,
            int const curr_index,
            int const predecessor_index,
            char const piece_type
        ) -> std::string {
            // char arrays are copied by value from what google says
            auto board_array_copy = board_array_representation;
            // do a swap where we move our piece from current location to new location
            board_array_copy[predecessor_index] = board_array_copy[curr_index];
            board_array_copy[curr_index] = piece_type;
            auto predecessor_FEN_string = convert_array_to_FEN(board_array_copy, isWhiteTurn);
            return predecessor_FEN_string;
        }


        auto generate_predecessor_board_states_for_pawns(
            chess::Piece& piece,
            chess::Square& sq,
            std::string& FEN_string,
            bool isWhiteTurn
        ) -> std::vector<std::string> {
            auto res = std::vector<std::string>{};

            auto board_array_representation = convert_FEN_to_array(FEN_string);
            auto curr_index = convert_square_to_index_for_array(sq);

            if (piece == chess::Piece::WHITEPAWN) {
                if (curr_index < 48) {
                    // unmoving pawn to square behind (below in chess notation for white)
                    if (board_array_representation[curr_index + 8] == '\0') {
                        auto temp = perform_unmove_or_uncapture(board_array_representation, isWhiteTurn, curr_index + 8, curr_index, '\0');
                        res.emplace_back(temp);
                    }

                    // uncapturing pawn to square that is below and to the left
                    if (((curr_index % 8) > 0) and board_array_representation[curr_index + 7] == '\0') {
                        for (auto piece_type : PIECE_TYPES_WITHOUT_KINGS) {
                            if (not piece_type_belongs_to_player(piece_type, isWhiteTurn)) {
                                auto temp = perform_unmove_or_uncapture(board_array_representation, isWhiteTurn, curr_index + 7, curr_index, piece_type);
                                res.emplace_back(temp);
                            }
                        }
                    }
                    // uncapturing pawn to square that is below and to the right
                    if (((curr_index % 8) < 7) and board_array_representation[curr_index + 9] == '\0') {
                        for (auto piece_type : PIECE_TYPES_WITHOUT_KINGS) {
                            if (not piece_type_belongs_to_player(piece_type, isWhiteTurn)) {
                                auto temp = perform_unmove_or_uncapture(board_array_representation, isWhiteTurn, curr_index + 9, curr_index, piece_type);
                                res.emplace_back(temp);
                            }
                        }
                    }
                }
            } else {
                if (curr_index > 15) {
                    // unmoving pawn to square behind (above in chess notation for black)
                    if (board_array_representation[curr_index - 8] == '\0') {
                        auto temp = perform_unmove_or_uncapture(board_array_representation, isWhiteTurn, curr_index - 8, curr_index, '\0');
                        res.emplace_back(temp);
                    }

                    // uncapturing pawn to square that is above and to the left
                    if (((curr_index % 8) > 0) and board_array_representation[curr_index - 9] == '\0') {
                        for (auto piece_type : PIECE_TYPES_WITHOUT_KINGS) {
                            if (not piece_type_belongs_to_player(piece_type, isWhiteTurn)) {
                                auto temp = perform_unmove_or_uncapture(board_array_representation, isWhiteTurn, curr_index - 9, curr_index, piece_type);
                                res.emplace_back(temp);
                            }
                        }
                    }
                    // uncapturing pawn to square that is below and to the right
                    if (((curr_index % 8) < 7) and board_array_representation[curr_index - 7] == '\0') {
                        for (auto piece_type : PIECE_TYPES_WITHOUT_KINGS) {
                            if (not piece_type_belongs_to_player(piece_type, isWhiteTurn)) {
                                auto temp = perform_unmove_or_uncapture(board_array_representation, isWhiteTurn, curr_index - 7, curr_index, piece_type);
                                res.emplace_back(temp);
                            }
                        }
                    }
                }
            }

            return res;
        }
    }


    // HEADER FUNCTION IMPLEMENTATIONS

    // Generates a list of piece combinations, all with size equal to the given number
    auto generate_piece_combinations_for_given_size(int const num_pieces) -> std::vector<std::vector<char>> {
        auto states = std::vector<std::vector<char>>{};
        states.emplace_back(std::vector<char>{'k', 'K'});
        for (auto i = 2; i < num_pieces; ++i) {
            auto temp = std::vector<std::vector<char>>{};
            for (auto j = states.begin(); j != states.end(); ++j) {
                // since we imposed an order on the pieces, we can do a trick to now generate
                // combinations instead of permutations
                for (auto k = find_iter_piece(j->back()); k != PIECE_TYPES_WITHOUT_KINGS.cend(); ++k) {
                    auto newest_state = std::vector<char>{j->begin(), j->end()};
                    newest_state.emplace_back(*k);
                    temp.emplace_back(newest_state);
                }
            }
            states = std::move(temp);
        }

        return states;
    }

    auto generate_piece_combinations(int const max_num_pieces) -> std::vector<std::vector<char>> {
        auto res = std::vector<std::vector<char>>{};
        for (auto i = 2; i <= max_num_pieces; ++i) {
            auto piece_combinations_for_size = generate_piece_combinations_for_given_size(i);
            for (auto j : piece_combinations_for_size) {
                res.emplace_back(j);
            }
        }
        return res;
    }

    auto generate_subsets_of_piece_combination(std::vector<char> const& max_piece_combination) -> std::vector<std::vector<char>> {
        auto res = std::vector<std::vector<char>>{};

        auto enumerator = 0;
        auto non_king_pieces = std::vector<char>{};

        std::copy_if(max_piece_combination.begin(), max_piece_combination.end(), std::back_inserter(non_king_pieces), [](char const& piece){
            return (piece != 'K' and piece != 'k');
        });

        // we use this as a bit mask that we increment each time, where the indices of the bit mask
        // represent the inclusion of pieces in the larger combination to produce all subsets
        for (auto enumerator = 0; enumerator < (1 << non_king_pieces.size()); ++enumerator) {
            auto curr_subset = std::vector<char>{'k', 'K'};
            for (auto i = 0; i < non_king_pieces.size(); ++i) {
                if ((enumerator >> i) & 1) {
                    curr_subset.emplace_back(non_king_pieces[i]);
                }
            }
            res.emplace_back(curr_subset);
        }

        return res;
    }

    // Now, for a given set of pieces we want to generate all possible board states
    // We filter our boards for checkmates for white, and remove any illegal/underfilled states
    auto generate_checkmates_for_piece_set_for_player(std::vector<char> const& pieces) -> std::vector<std::string> {
        auto checkmates_for_player = std::vector<std::string>{};

        // This lets us enumerate over the permutations of positions for each piece,
        // the uint8s represent the position of the piece on the board, and the index in our
        // enumerator correlates to its respective piece in the provided input vector of pieces
        auto enumerator = std::vector<uint8_t>(pieces.size(), 0);
        auto const max_enumerator_value = std::vector<uint8_t>(pieces.size(), NUM_BOARD_SQUARES - 1);
        auto counter = 0;
        // We can ignore the first and last cases for the enumerator because they are guaranteed to
        // have overlaps, and thus are underfilled
        while (enumerator != max_enumerator_value) {
            counter++;
            increment_enumerator(enumerator);
            if (contains_enumerator_overlaps(enumerator)) {
                continue;
            }

            auto board_array = std::array<char, NUM_BOARD_SQUARES>{};
            for (auto i = 0; i < pieces.size(); ++i) {
                board_array[enumerator[i]] = pieces[i];
            }

            auto const FEN_string = convert_array_to_FEN(board_array, false);
            auto board_state = chess::Board(FEN_string);
            if (is_legal_board_state(board_state) and is_checkmate_win_for_white(board_state)) {
                checkmates_for_player.emplace_back(FEN_string);
            }
        }

        return checkmates_for_player;
    }

    // Generates the direct predecessor board states for our current state, i.e. states where
    // a player takes one move to result in the current state (player turn matters)
    auto generate_predecessor_board_states(
        std::string const& FEN_string,
        bool const isWhiteTurn,
        int const max_pieces_present
    ) -> std::unordered_set<std::string> {
        auto predecessor_board_states = std::unordered_set<std::string>{};
        auto const curr = chess::Board(FEN_string);

        auto const occupied_spaces_bitboard = curr.occ();
        // bitboard of the player who just took a move, so we can find the squares of their pieces
        auto prev_turns_players_pieces_bitboard = curr.them(curr.sideToMove());
        auto const board_array_representation = convert_FEN_to_array(FEN_string);
        // we iterate over each piece of the player who just took a move, and undo the move
        while (prev_turns_players_pieces_bitboard.count()) {
            auto const sq = chess::Square(prev_turns_players_pieces_bitboard.pop());
            auto const curr_index = convert_square_to_index_for_array(sq);
            auto const piece = curr.at<chess::Piece>(sq);

            if ((piece == chess::Piece::WHITEPAWN) or (piece == chess::Piece::BLACKPAWN)) {
                // currently commenting out pawn logic due to the increases in time taken caused by
                // generating pawn unmoves, uncaptures and unpromotions

                // auto predecessor_FEN_strings = generate_predecessor_board_states_for_pawns(piece, sq, FEN_string, isWhiteTurn);
                // for (auto predecessor_FEN_string : predecessor_FEN_strings) {
                //     auto predecessor_board = chess::Board(predecessor_FEN_string);
                //     if (is_legal_board_state(predecessor_board)) {
                //         print_FEN_as_ASCII_board(predecessor_FEN_string);
                //         predecessor_board_states.emplace(predecessor_FEN_string);
                //     }
                // }
            } else {
                auto predecessor_locs_bitboard = get_piece_possible_predecessor_locations(piece, sq, occupied_spaces_bitboard);
                while (predecessor_locs_bitboard.count()) {
                    auto predecessor_index = convert_square_to_index_for_array(chess::Square(predecessor_locs_bitboard.pop()));
                    if (board_array_representation[predecessor_index] == '\0') {
                        if (occupied_spaces_bitboard.count() < max_pieces_present) {
                            for (auto piece_type : PIECE_TYPES_WITHOUT_KINGS) {
                                if (not piece_type_belongs_to_player(piece_type, isWhiteTurn)) {
                                    auto predecessor_FEN_string = perform_unmove_or_uncapture(board_array_representation, isWhiteTurn, curr_index, predecessor_index, piece_type);
                                    auto predecessor_board = chess::Board(predecessor_FEN_string);
                                    if (is_legal_board_state(predecessor_board)) {
                                        predecessor_board_states.emplace(predecessor_FEN_string);
                                    }
                                }
                            }
                        }

                        auto const predecessor_FEN_string = perform_unmove_or_uncapture(board_array_representation, isWhiteTurn, curr_index, predecessor_index, '\0');
                        auto predecessor_board = chess::Board(predecessor_FEN_string);
                        if (is_legal_board_state(predecessor_board)) {
                            predecessor_board_states.emplace(predecessor_FEN_string);
                        }
                    }
                }
            }
        }

        return predecessor_board_states;
    }

    auto generate_successor_boards(std::string const& curr_FEN) -> std::unordered_set<std::string> {
        auto successor_boards = std::unordered_set<std::string>{};

        auto const board = chess::Board(curr_FEN);
        auto movelist = chess::Movelist();
        chess::movegen::legalmoves(movelist, board);

        for (auto i : movelist) {
            auto successor_board = chess::Board(curr_FEN);
            successor_board.makeMove(i);
            successor_boards.emplace(board_to_FEN_wrapper(successor_board));
        }

        return successor_boards;
    }

    auto print_FEN_as_ASCII_board(std::string const& input) -> void {
        print_board_array_representation(convert_FEN_to_array(input));
    }

    auto board_to_FEN_wrapper(chess::Board const& board) -> std::string {
        auto array_representation = convert_FEN_to_array(board.getFen());
        return convert_array_to_FEN(array_representation, (board.sideToMove() == chess::Color::WHITE));
    }

    auto is_forced_win(std::string const& current_board, std::unordered_set<std::string> const& known_forced_wins) -> bool {
        for (auto curr_successor_board : generate_successor_boards(current_board)) {
            if (not known_forced_wins.contains(curr_successor_board)) {
                return false;
            }
        }

        return true;
    }

    auto get_depth_to_mate_for_state(
        std::string const& FEN_string,
        std::vector<std::unordered_set<std::string>> const& states_with_forceable_wins_for_white
    ) -> int {
        for (auto i = 0; i < states_with_forceable_wins_for_white.size(); ++i) {
            if (states_with_forceable_wins_for_white[i].contains(FEN_string)) {
                return i;
            }
        }

        return -1;
    }
}


#endif // COMP3821_PROJ_HELPER
