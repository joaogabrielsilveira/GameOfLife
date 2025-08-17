#include <fstream>
#include <iostream>
#include <unistd.h>
#include <vector>

#define BOARD_HEIGHT 5
#define BOARD_WIDTH 5

#define TRUE_VALUE '1'
#define FALSE_VALUE '0'

template <typename T>
class Matrix {
public:
    Matrix() : Matrix(0, 0) {};
    explicit Matrix(int n) : Matrix(n, n) {};

    Matrix(const int nLines, const int nCols) : _lines(nLines), _cols(nCols) {
        _data = std::vector(nLines * nCols, FALSE_VALUE);
    };

    T& operator()(const int line, const int col) {
        if (line < 0 || col < 0 || line >= _lines || col >= _cols) {
            throw std::out_of_range("Invalid matrix positions!");
        }
        return _data[line * _cols + col];
    };

    T const& operator()(const int line, const int col) const {
        if (line < 0 || col < 0 || line >= _lines || col >= _cols) {
            throw std::out_of_range("Invalid matrix positions!");
        }
        return _data[line * _cols + col];
    };

    void Print() const {
        for (int i = 0; i < _lines; i++) {
            for (int j = 0; j < _cols; j++) {
                std::cout << (*this)(i, j) << " ";
            }
            std::cout << "\n";
        }
    };

private:
    int _lines = 0;
    int _cols = 0;
    std::vector<T> _data;
};

template <typename T>
class GameBoard {
public:
    GameBoard() : _height(BOARD_HEIGHT), _width(BOARD_WIDTH) {
        _board = Matrix<T>(_height, _width);
    };

    GameBoard(const int height, const int width) : _height(height), _width(width) {
        _board = Matrix<T>(height, width);
    };

    ~GameBoard() = default;

    T& operator()(const int line, const int col) {
        return _board(line, col);
    };

    T const& operator()(const int line, const int col) const {
        return _board(line, col);
    };

    void SetTrue (const int line, const int col) {
        if (line < 0 || col < 0 || line >= _height || col >= _width) {
            throw std::out_of_range("Invalid board positions!");
        }
        _board(line, col) = TRUE_VALUE;
    };

    void SetFalse(const int line, const int col) {
        if (line < 0 || col < 0 || line >= _height || col >= _width) {
            throw std::out_of_range("Invalid board positions!");
        }
        _board(line, col) = FALSE_VALUE;
    };

    void Print() const {
        _board.Print();
    };

    [[nodiscard]] int CountNeighbors(const int line, const int col) const {
        int currentLine, currentCol;
        int counter = 0;

        for (int lDelta = -1; lDelta <= 1; lDelta++) {
            if (line + lDelta < 0) currentLine = _height - 1;
            else if (line + lDelta >= _height) currentLine = 0;
            else currentLine = line + lDelta;
            for (int cDelta = -1; cDelta <= 1; cDelta++) {
                if (col + cDelta < 0) currentCol = _width - 1;
                else if (col + cDelta >= _width) currentCol = 0;
                else currentCol = col + cDelta;

                if (_board(currentLine, currentCol) == TRUE_VALUE) {
                    if (currentLine != line || currentCol != col) counter++;
                }
            }
        }

        return counter;
    };

    void AdvanceBoardState() {
        Matrix<T> nextBoard(_height, _width);
        for (int line = 0; line < _height; line++) {
            for (int col = 0; col < _width; col++) {
                T currentValue = (*this)(line, col);
                const int currentNeighbors = CountNeighbors(line, col);
                if (currentValue == FALSE_VALUE) {
                    if (currentNeighbors == 3) {
                        nextBoard(line, col) = TRUE_VALUE;
                    }
                }
                else if (currentValue == TRUE_VALUE) {
                    if (currentNeighbors == 2) {
                        nextBoard(line, col) = TRUE_VALUE;
                    }
                }
            }
        }

        _board = nextBoard;
    };

private:
    int _height = BOARD_HEIGHT;
    int _width = BOARD_WIDTH;
    Matrix<T> _board;
};

void ReadFile(std::fstream& inputFile, GameBoard<char>& board) {
    int currentLine, currentCol;
    inputFile >> currentLine >> currentCol;
    board = GameBoard<char>(currentLine, currentCol);

    int liveCells;
    inputFile >> liveCells;

    for (int cell = 0; cell < liveCells; cell++) {
        inputFile >> currentLine >> currentCol;
        board.SetTrue(currentLine, currentCol);
    }
};

int main() {

    std::fstream inputFile("input.txt", std::fstream::in);
    if (!inputFile.is_open()) {
        throw std::runtime_error("Error opening input file!");
    }

    GameBoard<char> board;
    ReadFile(inputFile, board);

    board.Print();

    while (std::cin.get()) {
        board.AdvanceBoardState();
        board.Print();
    }

    return 0;
}