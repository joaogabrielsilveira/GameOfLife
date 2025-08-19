#include <fstream>
#include <iostream>
#include <vector>
#include <thread>
#include <SDL.h>

#define DEFAULT_BOARD_HEIGHT 800
#define DEFAULT_BOARD_WIDTH 600

#define TRUE_VALUE '1'
#define FALSE_VALUE '0'

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

template<typename T>
class GameBoard;

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
            throw std::out_of_range("Invalid matrix positions! Line: " + std::to_string(line)
                + ", Col: " + std::to_string(col));
        }
        return _data[line * _cols + col];
    };

    T const& operator()(const int line, const int col) const {
        if (line < 0 || col < 0 || line >= _lines || col >= _cols) {
            throw std::out_of_range("Invalid matrix positions!"
            " Line: " + std::to_string(line) + ", Col: " + std::to_string(col)
            + " (Borders: " + std::to_string(_lines - 1) + ", " + std::to_string(_cols - 1) + ")"
            );
        }
        return _data[line * _cols + col];
    };

    void Print() const {
        for (int i = 0; i < _lines; i++) {
            for (int j = 0; j < _cols; j++) {
                if ((*this)(i, j) == TRUE_VALUE) {
                    std::cout << " 🟩";
                }
                else {
                    std::cout << " ⬜";
                }
            }
            std::cout << "\n";
        }
    };

private:
    int _lines = 0;
    int _cols = 0;
    std::vector<T> _data;

    friend class GameBoard<T>;
};

template <typename T>
class GameBoard {
public:
    GameBoard() : _height(DEFAULT_BOARD_HEIGHT), _width(DEFAULT_BOARD_WIDTH), _currentCount(0), _totalCount(0)  {
        _board = Matrix<T>(_height, _width);
    };

    GameBoard(const int height, const int width) : _height(height), _width(width), _currentCount(0), _totalCount(0) {
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

    [[nodiscard]] int GetCurrentCount() const {
        return _currentCount;
    };

    [[nodiscard]] int GetTotalCount() const {
        return _totalCount;
    };

    [[nodiscard]] int GetLines() const {
        return _board._lines;
    }

    [[nodiscard]] int GetCols() const {
        return _board._cols;
    }

    void Render(SDL_Renderer* renderer, const int squareWid = 1, const int squareHei = 1) const {
        std::vector<SDL_Rect> liveSquares;
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
        for (int i = 0; i < _height; i++) {
            for (int j = 0; j < _width; j++) {
                if (_board(i,j) == TRUE_VALUE) {
                    SDL_Rect square = {j * squareWid, i * squareHei, squareWid, squareHei};
                    liveSquares.push_back(square);
                }
            }
        }
        for (auto& r : liveSquares) {
            std::cout << "renderizando " << r.x << " " << r.y << "\n";
        }
        SDL_RenderFillRects(renderer, liveSquares.data(), static_cast<int>(liveSquares.size()));
        SDL_RenderPresent(renderer);
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
        _currentCount = 0;
        Matrix<T> nextBoard(_height, _width);
        for (int line = 0; line < _height; line++) {
            for (int col = 0; col < _width; col++) {
                T currentValue = (*this)(line, col);
                const int currentNeighbors = CountNeighbors(line, col);
                if (currentValue == FALSE_VALUE) {
                    if (currentNeighbors == 3) {
                        _totalCount++;
                        nextBoard(line, col) = TRUE_VALUE;
                    }
                }
                else if (currentValue == TRUE_VALUE) {
                    _currentCount++;
                    if (currentNeighbors == 2 || currentNeighbors == 3) {
                        _totalCount++;
                        nextBoard(line, col) = TRUE_VALUE;
                    }
                }
            }
        }

        _board = nextBoard;
    };

private:
    int _height;
    int _width;
    Matrix<T> _board;
    int _currentCount;
    int _totalCount;
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

int main(const int argc, char ** argv) {

    int maxGenerations = 100;
    if (argc >= 3) maxGenerations = std::stoi(argv[2]);
    std::fstream inputFile(argv[1], std::fstream::in);
    if (!inputFile.is_open()) {
        throw std::runtime_error("Error opening input file!");
    }

    GameBoard<char> board;
    ReadFile(inputFile, board);

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow(
        "Game of Life",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN
        );
    SDL_Renderer* renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
        );

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    int generation = 0;
    bool isRunning = true;

    const int sizeRatioX = 800 / board.GetCols();
    const int sizeRatioY = 600 / board.GetLines();

    while (isRunning && ++generation <= maxGenerations) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                isRunning = false;
            }
        }
        if (window == nullptr) {
            std::cout << "\nGeneration " << generation << "\n";
        }
        else {
            std::string windowTitle = "Game of Life - Generation " + std::to_string(generation);
            SDL_SetWindowTitle(window, windowTitle.c_str());
        }
        if (renderer == nullptr) {
            board.Print();
        }
        else {
            board.Render(renderer, sizeRatioX, sizeRatioY);
        }
        board.AdvanceBoardState();
        std::this_thread::sleep_for(std::chrono_literals::operator ""ms(500));
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    std::cout << "A vida durou " << generation << " gerações, e terminou com " << board.GetCurrentCount()
    << " células vivas. No total, a vida foi criada " << board.GetTotalCount() << " vezes.\n";

    return 0;
}