#include <iostream>
#include <stdio.h>
#include <chess.hpp>
#include <stdint.h>
#include <array>
#include "./helper.cpp"
#include <string>

int main(int argc, char** argv) {
    int num_ending_pieces = 0;
    int max_pieces_present = 0;

    if (argc < 3) {
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
    } else {
        num_ending_pieces = std::stoi(argv[1]);
        max_pieces_present = std::stoi(argv[2]);
    }

    if (num_ending_pieces < 3 or num_ending_pieces > 5) {
        std::cout << "Error: Bad end piece count, please be greater than 2 and less than 5\n";
        return 1;
    } else if (num_ending_pieces > max_pieces_present) {
        std::cout << "Error: Bad max piece count, is less than end piece count\n";
        return 1;
    }


    // challenge is to enumerate a list of numbers (range 1-64) that is of length max_pieces_present
    // that covers all possible permutations. idk more elegant solution

    auto ending_states = helper::generate_piece_combinations(num_ending_pieces);
    for (auto i : ending_states) {
        for (auto j: i) {
            std::cout << j;
        }
        // listing the combinations of pieces that are generated (not counting positions yet)
        std::cout << '\n';

        helper::generate_checkmates_for_piece_set_for_player(i);
        break;
    }


    for (auto i = 0; i < num_ending_pieces; ++i) {
        // current idea, generate all pieces and project them into different locations
    }

    return 0;
}


