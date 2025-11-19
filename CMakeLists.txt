# ------------------------------------------------------------ #
# -------------- DO NOT TOUCH BELOW THIS LINE ---------------- #
# ------------------------------------------------------------ #

# this must be the first line of a CMake script.
# sets the lowerbound on what CMake version can be used.
cmake_minimum_required(VERSION 3.0)

# the name of this CMake project and what language it uses
# we could list more languages if we were using more.
project(COMP3821PROJ LANGUAGES CXX)

# we use C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS NO)

# this is helpful for editors like VS Code
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# helpful compiler flags for gcc/clang
# the descriptions for these flags can be found on the GNU Compiler Collection's webpage.
add_compile_options(
  -Wall
  -Wextra
  -Werror
  -pedantic-errors
  -Wconversion
  -Wsign-conversion
  -Wdouble-promotion
  -Wcast-align
  -Wformat=2
  -Wuninitialized
  -Wnull-dereference
  -Wnon-virtual-dtor
  -Woverloaded-virtual
  -Wdeprecated-copy-dtor
  -Wold-style-cast
  -Wzero-as-null-pointer-constant
  -Wsuggest-override
  -fstack-protector-strong
)

# debug builds should be compiled with sanitizers
# sanitizers are small libraries that check things like buffer overrun with minimal runtime overhead.
set(CMAKE_CXX_FLAGS_DEBUG_INIT "-fsanitize=address,undefined")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO_INIT "-fsanitize=address,undefined")
set(CMAKE_CXX_EXE_LINKER_FLAGS_DEBUG_INIT "-fsanitize=address,undefined")
set(CMAKE_CXX_EXE_LINKER_FLAGS_RELWITHDEBINFO_INIT "-fsanitize=address,undefined")


# # add the testing library Catch2
# enable_testing()
# add_library(catch2_main lib/catch2_main.cpp)
# target_include_directories(catch2_main PUBLIC lib)
# # link the library so that other programs can get it
# link_libraries(catch2_main)

# ------------------------------------------------------------ #
# -------------- DO NOT MODIFY ABOVE THIS LINE --------------- #
# ------------------------------------------------------------ #

enable_testing()
include_directories(${CMAKE_SOURCE_DIR}/external/catch2)


include_directories(${CMAKE_SOURCE_DIR}/external/chess-library/include)

add_library(helper src/helper.h src/helper.cpp)
link_libraries(helper)

add_executable(run_engine src/run_engine.cpp)
add_executable(get_next_move src/get_next_move.cpp)

add_executable(endgame_tablebase_test
    src/endgame_tablebase.test.cpp
    external/catch2_main.cpp
)

add_test(endgame_tablebase_test endgame_tablebase_test)

target_compile_options(run_engine PRIVATE
    -w
)

target_compile_options(get_next_move PRIVATE
    -w
)

target_compile_options(helper PRIVATE
    -w
)

target_compile_options(endgame_tablebase_test PRIVATE
    -w
)
