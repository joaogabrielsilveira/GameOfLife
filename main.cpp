#include <fstream>
#include <iostream>
#include <list>
#include <vector>
#include <thread>
#include <SDL.h>
#include <set>

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
            throw std::out_of_range("Invalid matrix positions!"
            " Line: " + std::to_string(line) + ", Col: " + std::to_string(col)
            + " (Borders: " + std::to_string(_lines - 1) + ", " + std::to_string(_cols - 1) + ")"
            );
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
    GameBoard() : _height(DEFAULT_BOARD_HEIGHT), _width(DEFAULT_BOARD_WIDTH), _totalCount(0)  {
        _board = Matrix<T>(_height, _width);
    };

    GameBoard(const int height, const int width) : _height(height), _width(width), _totalCount(0) {
        _board = Matrix<T>(height, width);
    };

    ~GameBoard() = default;

    T& operator()(const int line, const int col) {
        return _board(line, col);
    };

    T const& operator()(const int line, const int col) const {
        return _board(line, col);
    };

    void Print() const {
        _board.Print();
    };

    [[nodiscard]] int GetCurrentCount() const {
        return _lifePositions.size();
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
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

        for (auto& coord : _lifePositions) {
            const int x = coord.first;
            const int y = coord.second;
            SDL_Rect square = {y * squareWid, x * squareHei, squareWid, squareHei};
            liveSquares.push_back(square);
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
        _totalCount += _lifePositions.size();
        Matrix<T> nextBoard(_height, _width);
        for (int line = 0; line < _height; line++) {
            for (int col = 0; col < _width; col++) {
                T currentValue = (*this)(line, col);
                const int currentNeighbors = CountNeighbors(line, col);
                if (currentValue == FALSE_VALUE) {
                    if (currentNeighbors == 3) {
                        _totalCount++;
                        nextBoard(line, col) = TRUE_VALUE;
                        _lifePositions.emplace(line, col);
                    }
                }
                else if (currentValue == TRUE_VALUE) {
                    if (currentNeighbors == 2 || currentNeighbors == 3) {
                        _totalCount++;
                        nextBoard(line, col) = TRUE_VALUE;
                        _lifePositions.emplace(line, col);
                    }
                    else {
                        _lifePositions.erase(std::pair<int,int>(line, col));
                    }
                }
            }
        }

        _board = nextBoard;
    };

    void SaveToFile(std::ofstream& file) const {
        file << GetLines();
        file << " ";
        file << GetCols();
        file << std::endl;

        file << _lifePositions.size();
        file << std::endl;

        for (auto& coord : _lifePositions) {
            file << coord.first;
            file << " ";
            file << coord.second;
            file << std::endl;
        }
    }

    void CreateLife(const int line, const int col) {
        if (line < 0 || col < 0 || line >= _height || col >= _width) {
            throw std::out_of_range("Invalid board positions!");
        }
        _board(line, col) = TRUE_VALUE;
        _lifePositions.emplace(line, col);
    };

    void DestroyLife(const int line, const int col) {
        if (line < 0 || col < 0 || line >= _height || col >= _width) {
            throw std::out_of_range("Invalid board positions!");
        }
        _board(line, col) = FALSE_VALUE;
        _lifePositions.erase(std::pair<int,int>(line, col));
    };

private:
    int _height;
    int _width;
    Matrix<T> _board;
    std::set<std::pair<int, int>> _lifePositions = std::set<std::pair<int, int>>();
    int _totalCount;
};

void ReadFile(std::ifstream& inputFile, GameBoard<char>& board) {
    int currentLine, currentCol;
    inputFile >> currentLine >> currentCol;
    board = GameBoard<char>(currentLine, currentCol);

    int liveCells;
    inputFile >> liveCells;

    for (int cell = 0; cell < liveCells; cell++) {
        inputFile >> currentLine >> currentCol;
        board.CreateLife(currentLine, currentCol);
    }
};

int main(const int argc, char ** argv) {
    int maxGenerations = 1000000;
    if (argc >= 3) maxGenerations = std::stoi(argv[2]);

    std::ifstream inputFile(argv[1]);
    if (!inputFile.is_open()) {
        throw std::runtime_error("Error opening input file!");
    }

    int fileCount = 0;
    std::ifstream fileCounterIn("fileCounter.txt");
    if (!fileCounterIn.is_open()) {
        throw std::runtime_error("Error opening count file!");
    }
    fileCounterIn >> fileCount;

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

    const int sizeRatioX = WINDOW_WIDTH / board.GetCols();
    const int sizeRatioY = WINDOW_HEIGHT / board.GetLines();

    const double srxF = static_cast<double>(WINDOW_WIDTH) / board.GetCols();
    const double sryF = static_cast<double>(WINDOW_HEIGHT) / board.GetLines();

    if (ceil(srxF) > sizeRatioX && ceil(sryF) > sizeRatioY) {
        SDL_SetWindowSize(window, board.GetCols() * sizeRatioX, board.GetLines() * sizeRatioY);
    }

    else if (ceil(sryF) > sizeRatioY) {
        SDL_SetWindowSize(window, WINDOW_WIDTH, board.GetLines() * sizeRatioY);
    }

    else if (ceil(srxF) > sizeRatioX) {
        SDL_SetWindowSize(window, board.GetCols() * sizeRatioX, WINDOW_HEIGHT);
    }

    bool paused = false;
    bool mouseHeldLeft = false;
    bool mouseHeldRight = false;

    while (isRunning && generation <= maxGenerations) {
        if (!paused) {
            generation++;
        }
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    isRunning = false;
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        int mouseX = event.button.x;
                        int mouseY = event.button.y;
                        if (mouseX < WINDOW_WIDTH && mouseY < WINDOW_HEIGHT) {
                            board.CreateLife(mouseY / sizeRatioY, mouseX / sizeRatioX);
                        }
                        mouseHeldLeft = true;
                    }
                    else if (event.button.button == SDL_BUTTON_RIGHT) {
                        int mouseX = event.button.x;
                        int mouseY = event.button.y;
                        if (mouseX < WINDOW_WIDTH && mouseY < WINDOW_HEIGHT) {
                            board.CreateLife(mouseY / sizeRatioY, mouseX / sizeRatioX);
                        }
                        mouseHeldRight = true;
                    }
                    break;
                case SDL_MOUSEBUTTONUP:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        mouseHeldLeft = false;
                    }
                    if (event.button.button == SDL_BUTTON_RIGHT) {
                        mouseHeldRight = false;
                    }
                    break;
                case SDL_MOUSEMOTION:
                    if (mouseHeldLeft) {
                        int mouseX = event.motion.x;
                        int mouseY = event.motion.y;
                        if (mouseX < WINDOW_WIDTH && mouseY < WINDOW_HEIGHT) {
                            board.CreateLife(mouseY / sizeRatioY, mouseX / sizeRatioX);
                        }
                    }

                    else if (mouseHeldRight) {
                        int mouseX = event.motion.x;
                        int mouseY = event.motion.y;
                        if (mouseX < WINDOW_WIDTH && mouseY < WINDOW_HEIGHT) {
                            board.DestroyLife(mouseY / sizeRatioY, mouseX / sizeRatioX);
                        }
                    }
                    break;
                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_SPACE) {
                        paused = !paused;
                    }
                    if (event.key.keysym.sym == SDLK_BACKSPACE) {
                        fileCount++;
                        std::ofstream outputFile("saves/saved_file_" + std::to_string(fileCount) + ".txt");
                        board.SaveToFile(outputFile);
                        outputFile.close();
                    }
                    break;
                default:
                    break;
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
        if (!paused) {
            board.AdvanceBoardState();
        }
        std::this_thread::sleep_for(std::chrono_literals::operator ""ms(100));
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    fileCounterIn.close();
    
    std::ofstream fileCounterOut("fileCounter.txt");
    if (!fileCounterOut.is_open()) {
        throw std::runtime_error("Error opening count file!");
    }
    fileCounterOut << fileCount;

    fileCounterOut.close();
    inputFile.close();

    std::cout << "A simulação durou por " << generation << " gerações, e terminou com " << board.GetCurrentCount()
    << " células vivas. No total, a vida foi criada " << board.GetTotalCount() << " vezes.\n";

    return 0;
}