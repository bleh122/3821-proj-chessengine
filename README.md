# COMP3821 CHESS ENGINE PROJECT REPOSITORY

This is our COMP3821 group's repository for the code that we will submit for our project, which focuses on implementing algorithms for endgame tablebase generation and chess solvability.

The project has mainly been built in C++20 so far, and utilises the chess library from https://github.com/Disservin/chess-library for board storage and move generation.

To build the project, run the following commands:
```bash
git clone https://github.com/bleh122/3821-proj-chessengine.git
cd 3821-proj-chessengine
cmake -DCMAKE_BUILD_TYPE=Release -B build
cmake -B build
cd build
make
./run_engine
```

The above will generate an `output.csv` file in the build directory, which stores the results/tablebase from the engine. These results can be queried to find the optimal move for the current board state (if it was reachable from the parameters provided to the earlier program) by running a separate program:
(assuming we are still in /build)
```bash
./gen_next_move
```
This command accepts string input of FEN notation for the position of pieces on the board (the section similar to 8/8/8/8/8/8/8/8, and nothing else) with the assumption that the player is on the white side (if playing for black, then invert the colours of pieces) with the current turn being for the white player.
