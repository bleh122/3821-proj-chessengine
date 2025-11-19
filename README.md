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

Doing so will cause `run_engine` to print its instructions for its use, namely the provision of the three command line arguments to it:
- max_depth_to_mate: an integer for the max depth to mate we wish to check.
- max_num_pieces: an integer for the max number of pieces we wish to test for. This value should range between 2 (min legal number of pieces in a chess game) to 4 (likely the highest value for which we our implementation will have enough space/time to run, 5 may be possible depending on hardware).
- starting_pieces: an optional string (can be left empty) containing pieces from FEN notation without spaces (e.g. KkQqRrNnBb). If provided, the length of this string should be equal to max_num_pieces, and if not provided, then all possible groups of pieces up to max_num_pieces will be tested.


One example to test with is `./run_engine 5 4 kKQn`, which will determine which boards have depth to mates of less than 5 for the piece set (benchmarks of real 1m20.853s according to linux's time utility on a 3.2ghz 8 core processor, when built in release mode) with a 35MB output file.


The above will generate an `output.csv` file in the build directory, which stores the results/tablebase from the engine. These results can be queried to find the optimal move for the current board state (if it was reachable from the parameters provided to the earlier program) by running a separate program:
(assuming we are still in /build)
```bash
./gen_next_move
```
This command accepts string input of FEN notation for the position of pieces on the board (the section similar to 8/8/8/8/8/8/8/8, and nothing else) with the assumption that the player is on the white side (if playing for black, then invert the colours of pieces) with the current turn being for the white player.
