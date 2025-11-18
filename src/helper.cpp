#ifndef COMP3821_PROJ_HELPER
#define COMP3821_PROJ_HELPER


#include <vector>
#include <set>
#include <algorithm>
#include <string>
#include <chess.hpp>
#include <unordered_set>
#include <numeric>

namespace helper {
    // We use FEN strings to represent the chess board states as we input them into chess-library.
    // We treat our player as the White player (since it doesn't matter which colour they are, only
    // whether it is their turn or not) and the opposite for the opponent.
    // We assume a board side length of 8, making the current code rather brittle to solving chess
    // games with larger boards.
    // We currently assume that endgames do not include pawns due to computational constraints

    // private functions
    namespace {
        // Pieces are represented as characters in this representation, we ignore pawns currently
        auto const piece_types_without_kings = std::set<char>{'B', 'N', 'Q', 'R', 'b', 'n', 'q', 'r'};
        // auto const piece_types_without_kings = std::set<char>{'B', 'N', 'P', 'Q', 'R', 'b', 'n', 'p', 'q', 'r'};

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

    auto generate_subsets_of_piece_combination(std::vector<char>& max_piece_combination) -> std::vector<std::vector<char>> {
        auto res = std::vector<std::vector<char>>{};

        int enumerator = 0;
        auto non_king_pieces = std::vector<char>{};
        std::cout << "test\n";

        std::copy_if(max_piece_combination.begin(), max_piece_combination.end(), std::back_inserter(non_king_pieces), [](char& piece){
            return (piece != 'K' and piece != 'k');
        });

        // we use this as a bit mask that we increment each time, where the indices of the bit mask
        // represent the inclusion of pieces in the larger combination to produce all subsets
        for (auto enumerator = 1; enumerator < (1 << non_king_pieces.size()); ++enumerator) {
            auto curr_subset = std::vector<char>{'k', 'K'};
            for (auto i = 0; i < non_king_pieces.size(); ++i) {
                if (enumerator >> i) {
                    curr_subset.emplace_back(non_king_pieces[i]);
                }
            }
            res.emplace_back(curr_subset);
        }

        return res;
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

        auto contains_enumerator_overlaps(std::vector<uint8_t> input) -> bool {
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

        auto convert_array_to_FEN(
            std::array<char, 1 + num_board_squares_minus_one>& board,
            bool isWhiteTurn
        ) -> std::string {
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
            FEN_string.append(isWhiteTurn ? " w - - 0 1" : " b - - 0 1");

            return FEN_string;
        }

        // assuming that both players have kings (guaranteed from earlier code)
        auto is_legal_board_state(chess::Board& board) -> bool {
            // check if king of player who just made move is in check (and thus if game is illegal)
            board.makeNullMove();
            if (board.inCheck()) {
                return false;
            }
            board.makeNullMove();
            return true;
        }

        // assuming that board state is legal, and current turn is black
        auto is_checkmate_win_for_white(chess::Board& board) -> bool {
            return board.inCheck() and (board.isGameOver().first == chess::GameResultReason::CHECKMATE);
        }
    }

    // Now, for a given set of pieces we want to generate all possible board states
    // We filter our boards for checkmates for white, and remove any illegal/underfilled states
    auto generate_checkmates_for_piece_set_for_player(std::vector<char> const& pieces) -> std::vector<std::string> {
        auto checkmates_for_player = std::vector<std::string>{};

        // this lets us enumerate over the permutations of positions for each piece,
        // the uint8s represent the position of the piece on the board, and the index in our
        // enumerator correlates to its respective piece in the provided input vector of pieces
        auto enumerator = std::vector<uint8_t>(pieces.size(), 0);
        auto const max_enumerator_value = std::vector<uint8_t>(pieces.size(), num_board_squares_minus_one);

        // we can ignore the first and last cases for the enumerator because they are guaranteed to
        // have overlaps, and thus are underfilled
        while (enumerator != max_enumerator_value) {
            increment_enumerator(enumerator);
            if (contains_enumerator_overlaps(enumerator)) {
                continue;
            }

            auto board_array = std::array<char, 1 + num_board_squares_minus_one>{};
            for (auto i = 0; i < pieces.size(); ++i) {
                board_array[enumerator[i]] = pieces[i];
            }

            // FEN string is for a board state where it is black's turn
            auto FEN_string = convert_array_to_FEN(board_array, false);
            auto board_state = chess::Board(FEN_string);
            if (is_legal_board_state(board_state) and is_checkmate_win_for_white(board_state)) {
                checkmates_for_player.emplace_back(FEN_string);
            }
            // break;
        }

        return checkmates_for_player;
    }

    // private functions
    namespace {
        // have to serialise back to an array to make it easy to create a new board
        // because undoing illegal moves is undefined behaviour in chess-library
        // (we don't have a checker) for that right now
        auto convert_FEN_to_array(std::string const& FEN) -> std::array<char, 1 + num_board_squares_minus_one>  {
            auto result_array = std::array<char, 1 + num_board_squares_minus_one>{};
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

        auto print_board_array_representation(std::array<char, 1 + num_board_squares_minus_one>& board) {
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

        // chess notation has different order of counting for rank (row), so to convert it to our
        // contiguous array indices we have to translate it like so
        auto convert_square_to_index_for_array(chess::Square& sq) -> int {
            int row_offset = 7 - sq.rank();
            int col_offset = sq.file();

            return (row_offset * 8) + col_offset;
        }

        auto convert_array_index_to_square(int index) -> chess::Square {
            int rank = 7 - (index / 8);
            int file = index % 8;
            return chess::Square(chess::Rank(rank), chess::File(file));
        }

        auto get_piece_possible_predecessor_locations(
            chess::Piece& piece,
            chess::Square& current_position,
            chess::Bitboard& current_occupied_spaces
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
            std::array<char, 1 + num_board_squares_minus_one>& board_array_representation,
            bool isWhiteTurn,
            int curr_index,
            int predecessor_index,
            char piece_type
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
                        for (auto piece_type : piece_types_without_kings) {
                            if (not piece_type_belongs_to_player(piece_type, isWhiteTurn)) {
                                auto temp = perform_unmove_or_uncapture(board_array_representation, isWhiteTurn, curr_index + 7, curr_index, piece_type);
                                res.emplace_back(temp);
                            }
                        }
                    }
                    // uncapturing pawn to square that is below and to the right
                    if (((curr_index % 8) < 7) and board_array_representation[curr_index + 9] == '\0') {
                        for (auto piece_type : piece_types_without_kings) {
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
                        for (auto piece_type : piece_types_without_kings) {
                            if (not piece_type_belongs_to_player(piece_type, isWhiteTurn)) {
                                auto temp = perform_unmove_or_uncapture(board_array_representation, isWhiteTurn, curr_index - 9, curr_index, piece_type);
                                res.emplace_back(temp);
                            }
                        }
                    }
                    // uncapturing pawn to square that is below and to the right
                    if (((curr_index % 8) < 7) and board_array_representation[curr_index - 7] == '\0') {
                        for (auto piece_type : piece_types_without_kings) {
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

    auto print_FEN_as_ASCII_board(std::string& input) -> void;


    // Generates the direct predecessor board states for our current state, i.e. states where
    // a player takes one move to result in the current state (player turn matters)
    auto generate_predecessor_board_states(
        std::string& FEN_string,
        bool isWhiteTurn,
        int max_pieces_present
    ) -> std::unordered_set<std::string> {
        auto predecessor_board_states = std::unordered_set<std::string>{};
        auto curr = chess::Board(FEN_string);

        auto occupied_spaces_bitboard = curr.occ();
        // bitboard of the player who just took a move, so we can find the squares of their pieces
        auto prev_turns_players_pieces_bitboard = curr.them(curr.sideToMove());
        auto board_array_representation = convert_FEN_to_array(FEN_string);

        // we iterate over each piece of the player who just took a move, and undo the move
        while (prev_turns_players_pieces_bitboard.count()) {
            auto lsb_position = prev_turns_players_pieces_bitboard.pop();
            auto sq = chess::Square(lsb_position);
            auto curr_index = convert_square_to_index_for_array(sq);
            auto piece = curr.at<chess::Piece>(sq);

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
                    auto lsb_predecessor_position = predecessor_locs_bitboard.pop();
                    auto sq_predecessor = chess::Square(lsb_predecessor_position);

                    auto predecessor_index = convert_square_to_index_for_array(sq_predecessor);
                    if (board_array_representation[predecessor_index] == '\0') {
                        if (occupied_spaces_bitboard.count() < max_pieces_present) {
                            for (auto piece_type : piece_types_without_kings) {
                                if (not piece_type_belongs_to_player(piece_type, isWhiteTurn)) {
                                    auto predecessor_FEN_string = perform_unmove_or_uncapture(board_array_representation, isWhiteTurn, curr_index, predecessor_index, piece_type);
                                    auto predecessor_board = chess::Board(predecessor_FEN_string);
                                    if (is_legal_board_state(predecessor_board)) {
                                        predecessor_board_states.emplace(predecessor_FEN_string);
                                    }
                                }
                            }
                        }
                        auto predecessor_FEN_string = perform_unmove_or_uncapture(board_array_representation, isWhiteTurn, curr_index, predecessor_index, '\0');
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

    namespace {
        auto board_to_FEN_wrapper(chess::Board& board) -> std::string {
            auto temp = convert_FEN_to_array(board.getFen());
            return convert_array_to_FEN(temp, (board.sideToMove() == chess::Color::WHITE));
        }
    }

    auto generate_successor_boards(std::string& curr_FEN) -> std::unordered_set<std::string> {
        auto successor_boards = std::unordered_set<std::string>{};

        auto board = chess::Board(curr_FEN);
        auto movelist = chess::Movelist();
        chess::movegen::legalmoves(movelist, board);

        for (auto i : movelist) {
            auto successor_board = chess::Board(curr_FEN);
            successor_board.makeMove(i);
            successor_boards.emplace(board_to_FEN_wrapper(successor_board));
        }

        return successor_boards;
    }

    auto print_FEN_as_ASCII_board(std::string& input) -> void {
        auto board = convert_FEN_to_array(input);
        print_board_array_representation(board);
    }

    // If every successor board to our current board is already known as a forced win, then this is
    // a state where we can force a win (a state can go from returning false to returning true
    // upon repeated queries as our set of known forced wins increases)
    auto is_forced_win(std::string& current_board, std::unordered_set<std::string> const& known_forced_wins) -> bool {
        auto successor_boards = generate_successor_boards(current_board);

        for (auto i : successor_boards) {
            if (not known_forced_wins.contains(i)) {
                return false;
            }
        }

        return true;
    }
}


#endif // COMP3821_PROJ_HELPER
