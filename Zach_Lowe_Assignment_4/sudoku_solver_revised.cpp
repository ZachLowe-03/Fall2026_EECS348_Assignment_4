/*
 * EECS 348 Assignment 4
 * Program Name : sudoku_solver_revised.cpp
 * Description  : Object-oriented, recursive brute-force Sudoku solver. For each
 *                puzzle file it prints the file name, the original puzzle, and
 *                EVERY solution found (or "No solution found").
 * Inputs       : puzzle1.txt ... puzzle5.txt (9 rows of 9 tokens; digits 1-9,
 *                '_' for a blank cell), or file names given on the command line.
 * Output       : Console output: file name, puzzle, solution(s), solution count.
 * Collaborators: Claude
 *                Code generated with help from Claude (Anthropic GenAI).
 * Author       : Zach Lowe (Prompted and revised with help from Claude)
 * Created      : 10/14/2026   Last revised: 10/15/2026
 * Revisions    : 10/15/2026 - Revised with help from Claude to add strict input
 *                validation in load() with row/column error messages, and a final
 *                solution check before printing.
 */

#include <iostream>   // for std::cout / std::cerr output
#include <fstream>    // for reading puzzle files
#include <sstream>    // for splitting lines into tokens
#include <string>     // for std::string
#include <vector>     // for std::vector (list of candidates)
 
// Class that represents one Sudoku board and knows how to solve itself.
class SudokuPuzzle {
private:
    static const int N = 9;        // board is N x N
    static const int BOX = 3;      // each sub-grid is BOX x BOX
    int grid[N][N];                // 0 means empty, 1-9 are filled values
    long solutionCount;            // number of solutions found so far
 
    // Collects the digits that can legally go in cell (row, col).
    std::vector<int> candidates(int row, int col) const {
        bool used[N + 1] = {false};                    // used[d] true if digit d is blocked
        for (int i = 0; i < N; ++i) {                  // scan the row and the column
            used[grid[row][i]] = true;                 // digit already in this row
            used[grid[i][col]] = true;                 // digit already in this column
        }
        int br = (row / BOX) * BOX;                    // top row of this 3x3 box
        int bc = (col / BOX) * BOX;                    // left column of this 3x3 box
        for (int r = 0; r < BOX; ++r)                  // scan the 3x3 box
            for (int c = 0; c < BOX; ++c)
                used[grid[br + r][bc + c]] = true;     // digit already in this box
        std::vector<int> result;                       // list of allowed digits
        for (int d = 1; d <= N; ++d)                   // try every digit 1-9
            if (!used[d]) result.push_back(d);         // keep the ones not blocked
        return result;                                 // may be empty -> dead end
    }
 
    // Finds the first empty cell; returns false if the board is full.
    bool findEmpty(int &row, int &col) const {
        for (int r = 0; r < N; ++r)                    // go through rows
            for (int c = 0; c < N; ++c)                // go through columns
                if (grid[r][c] == 0) {                 // empty cell found
                    row = r;                           // report its row
                    col = c;                           // report its column
                    return true;                       // an empty cell exists
                }
        return false;                                  // no empty cells: k == 81
    }
 
    // Checks that no filled cell repeats a digit in its row, column, or box.
    // Blanks are ignored, so this works on both a partial and a finished board.
    bool isConsistent() const {
        for (int r = 0; r < N; ++r)                    // check every cell
            for (int c = 0; c < N; ++c) {
                int v = grid[r][c];                    // value in this cell
                if (v == 0) continue;                  // skip blanks
                for (int i = 0; i < N; ++i) {          // compare with row/column mates
                    if (i != c && grid[r][i] == v) return false;  // duplicate in row
                    if (i != r && grid[i][c] == v) return false;  // duplicate in column
                }
                int br = (r / BOX) * BOX;              // box origin row
                int bc = (c / BOX) * BOX;              // box origin column
                for (int rr = br; rr < br + BOX; ++rr)         // compare within box
                    for (int cc = bc; cc < bc + BOX; ++cc)
                        if ((rr != r || cc != c) && grid[rr][cc] == v)
                            return false;              // duplicate in box
            }
        return true;                                   // no conflicts found
    }
 
    // A board is a valid solution only if it has no blanks and breaks no rule.
    bool isValidSolution() const {
        int row = 0, col = 0;                          // findEmpty needs somewhere to report
        return !findEmpty(row, col) && isConsistent(); // full board with no duplicates
    }
 
    // Recursive brute-force search. Each call fills one more empty cell.
    void solve() {
        int row = 0, col = 0;                          // location of the next empty cell
        if (!findEmpty(row, col)) {                    // base case: all 81 cells filled
            if (!isValidSolution()) {                  // safeguard: should never happen
                std::cerr << "Internal error: completed board is invalid; not printed.\n";
                return;                                // skip it rather than print a bad answer
            }
            ++solutionCount;                           // count this solution
            std::cout << "Solution " << solutionCount << ":\n";  // label it
            print();                                   // print the solved board
            return;                                    // go back to look for more
        }
        std::vector<int> options = candidates(row, col);  // legal digits for this cell
        for (size_t i = 0; i < options.size(); ++i) {  // try each candidate in order
            grid[row][col] = options[i];               // place the candidate
            solve();                                   // recurse with k+1 cells filled
            grid[row][col] = 0;                        // backtrack: undo the placement
        }
        // If options was empty, or all failed, we simply return (no solution on this path).
    }
 
public:
    // Constructor: start with an empty board and zero solutions.
    SudokuPuzzle() : solutionCount(0) {
        for (int r = 0; r < N; ++r)                    // clear every row
            for (int c = 0; c < N; ++c)
                grid[r][c] = 0;                        // 0 = empty
    }

    // Reads the board from a file. Every token must be exactly one character:
    // a digit 1-9 or '_'. On failure returns false and sets error to a message
    // naming the offending row and column.
    bool load(const std::string &filename, std::string &error) {
        std::ifstream in(filename.c_str());            // open the file
        if (!in) {
            error = "could not open file";
            return false;
        }
        for (int r = 0; r < N; ++r)                    // read 9 rows
            for (int c = 0; c < N; ++c) {              // of 9 tokens each
                std::ostringstream where;              // location text for error messages
                where << "row " << (r + 1) << ", column " << (c + 1);
                std::string tok;                       // one whitespace-separated token
                if (!(in >> tok)) {
                    error = "file ended early; expected 81 cells, missing at " + where.str();
                    return false;
                }
                if (tok.size() != 1 || (tok[0] != '_' && (tok[0] < '1' || tok[0] > '9'))) {
                    error = "invalid cell \"" + tok + "\" at " + where.str() +
                            " (expected 1-9 or _)";
                    return false;
                }
                grid[r][c] = (tok[0] == '_') ? 0 : (tok[0] - '0');  // '_' -> 0, else digit
            }
        std::string extra;                             // anything left over means a bad file
        if (in >> extra) {
            error = "more than 81 cells; unexpected extra token \"" + extra + "\"";
            return false;
        }
        return true;                                   // board loaded
    }
 
    // Prints the board using the same format as the input files.
    void print() const {
        for (int r = 0; r < N; ++r) {                  // each row
            for (int c = 0; c < N; ++c) {              // each cell
                if (grid[r][c] == 0) std::cout << "_ ";            // blank cell
                else std::cout << grid[r][c] << " ";               // filled cell
            }
            std::cout << "\n";                         // end of row
        }
        std::cout << "\n";                             // blank line after the board
    }
 
    // Public entry point: runs the search and reports the results.
    void solveAll() {
        solutionCount = 0;                             // reset the counter
        if (isConsistent()) solve();                   // only search if clues are consistent
        if (solutionCount == 0)                        // nothing found
            std::cout << "No solution found\n\n";      // report failure
        else
            std::cout << "Solutions found: " << solutionCount << "\n\n";  // report count
    }
};

int main(int argc, char *argv[]) {
    std::vector<std::string> files;                    // list of puzzle files to process
    if (argc > 1) {                                    // use command-line names if given
        for (int i = 1; i < argc; ++i) files.push_back(argv[i]);
    } else {                                           // otherwise default to the five files
        for (int i = 1; i <= 5; ++i) {
            std::ostringstream name;                   // build "puzzleN.txt"
            name << "puzzle" << i << ".txt";
            files.push_back(name.str());
        }
    }
    for (size_t i = 0; i < files.size(); ++i) {        // process each file
        std::cout << "==== " << files[i] << " ====\n";  // print the file name
        SudokuPuzzle puzzle;                           // create a puzzle object
        std::string error;                             // reason a load failed, if it does
        if (!puzzle.load(files[i], error)) {           // try to read the file
            std::cout << "Error in " << files[i] << ": " << error << "\n\n";
            continue;                                  // move on to the next file
        }
        std::cout << "Puzzle:\n";                      // label the original puzzle
        puzzle.print();                                // show the puzzle
        puzzle.solveAll();                             // solve and print all solutions
    }
    return 0;                                          // success
}
