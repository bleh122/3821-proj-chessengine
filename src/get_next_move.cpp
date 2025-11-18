#include <stdio.h>
#include <fstream>
#include <chess.hpp>
#include "helper.cpp"
#include <map>

auto convert_components_to_FEN(std::string& FEN_position, std::string& player_turn) -> std::string {
    auto res = FEN_position;
    res.push_back(' ');
    res.append(player_turn);
    res.append(" - - 0 1");
    return res;
}

auto get_depth_to_mate_for_state(
    std::string& FEN_string,
    std::vector<std::unordered_set<std::string>> states_with_forceable_wins_for_white
) -> int {
    for (auto i = 0; i < states_with_forceable_wins_for_white.size(); ++i) {
        if (states_with_forceable_wins_for_white[i].contains(FEN_string)) {
            return i;
        }
    }

    return -1;
}

auto get_curr_board_FEN() {
    std::string FEN_string;
    std::string turn = "w";

    std::cout   << "\n\nPlease enter a FEN string with the information of the current board " 
                << "(it must always be our player's turn, with our player on the white side, "
                << "only piece positions are required, e.g. "
                << "'8/1k6/8/4K3/8/8/Q7/8' as the turn being [w] for the white player is implied): ";
    if (not (std::cin >> FEN_string)) {
        std::cout << "Error: string not entered.\n";
        std::abort();
    }

    return convert_components_to_FEN(FEN_string, turn);
}

auto convert_piece_to_string = std::map<chess::Piece, std::string>({
    {chess::Piece::WHITEBISHOP, "bishop"},
    {chess::Piece::WHITEKING, "king"},
    {chess::Piece::WHITEQUEEN, "queen"},
    {chess::Piece::WHITEROOK, "rook"},
    {chess::Piece::WHITEKNIGHT, "knight"},
    {chess::Piece::WHITEPAWN, "pawn"},
});

int main(void) {
    auto FEN_string = get_curr_board_FEN();

    // deserialise output.csv
    auto input_file = std::ifstream("output.csv");

    auto states_with_forceable_wins_for_white = std::vector<std::unordered_set<std::string>>{};

    int depth_to_mate;
    while (input_file >> depth_to_mate) {
        std::string curr_FEN_position;
        std::string curr_player_turn;
        input_file >> curr_FEN_position;
        input_file >> curr_player_turn;
        while (depth_to_mate >= states_with_forceable_wins_for_white.size()) {
            states_with_forceable_wins_for_white.emplace_back(std::unordered_set<std::string>{});
        }
        auto curr_FEN = convert_components_to_FEN(curr_FEN_position, curr_player_turn);
        states_with_forceable_wins_for_white[depth_to_mate].emplace(curr_FEN);
    }

    while (depth_to_mate = get_depth_to_mate_for_state(FEN_string, states_with_forceable_wins_for_white)) {
        if (depth_to_mate == -1) {
            std::cout << "There is no forced win for this board state according to our current "
                << "endgame tablebase.\n";
            return 0;
        }

        auto board = chess::Board(FEN_string);

        auto movelist = chess::Movelist();
        chess::movegen::legalmoves(movelist, board);

        for (auto curr_move : movelist) {
            auto successor_board = chess::Board(FEN_string);
            successor_board.makeMove(curr_move);
            auto successor_FEN = helper::board_to_FEN_wrapper(successor_board);

            if (states_with_forceable_wins_for_white[depth_to_mate - 1].contains(successor_FEN)) {
                std::cout << "Move the " << convert_piece_to_string[board.at(curr_move.from())]
                    << " from " << curr_move.from() << " to " << curr_move.to() << ".\n";

                std::cout << "\n\nNow wait for the opponent to take their own move, then continue.\n\n";

                FEN_string = get_curr_board_FEN();
            }
        }
    }

    std::cout << "Congrats for using this tablebase to checkmate.";

    return 0;
}