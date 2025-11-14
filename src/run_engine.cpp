#include <iostream>
#include <stdio.h>
#include <chess.hpp>
#include <stdint.h>
#include <array>

int main(void) {
    int num_ending_pieces;
    int max_pieces_present;

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

    if (num_ending_pieces < 3) {
        std::cout << "Error: Bad end piece count, please be greater than 2\n";
        return 1;
    } else if (num_ending_pieces > max_pieces_present) {
        std::cout << "Error: Bad max piece count, is less than end piece count\n";
        return 1;
    }

    // challenge is to enumerate a list of numbers (range 1-64) that is of length max_pieces_present
    // that covers all possible permutations. idk more elegant solution
    for (auto i = 0; i < num_ending_pieces; ++i) {
    }

    return 0;
}


