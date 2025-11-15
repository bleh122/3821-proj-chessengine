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
