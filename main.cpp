#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>
#include <thread>
#include <SDL.h>
#include <SDL_ttf.h>
#include <set>

#define DEFAULT_BOARD_HEIGHT 800
#define DEFAULT_BOARD_WIDTH 600
#define FONT_SIZE 24

#define TRUE_VALUE '1'
#define FALSE_VALUE '0'

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

const std::string savesPath = "saves";
const std::string fontPath = "OpenSans.ttf";
constexpr SDL_Color textColor = {255, 255, 255, 255};

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
    GameBoard() : _height(DEFAULT_BOARD_HEIGHT), _width(DEFAULT_BOARD_WIDTH), _totalCount(0),
    _board(Matrix<T>(_height, _width)), _lifePositions(std::set<std::pair<int, int>>()) {};

    GameBoard(const int height, const int width) : _height(height), _width(width), _totalCount(0),
    _board (Matrix<T>(height, width)), _lifePositions(std::set<std::pair<int, int>>()) {};

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
        _totalCount++;
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
    int _totalCount;
    Matrix<T> _board;
    std::set<std::pair<int, int>> _lifePositions;
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

int GetNumber() {
    int n;
    while (!(std::cin >> n)) {
        std::cout << "Entrada inválida, insira um número\n";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    return n;
};

void DrawText(SDL_Renderer* renderer, const SDL_Rect& rect, const std::string& text, TTF_Font* textFont, const SDL_Color& color) {
    SDL_Surface* textSurface = TTF_RenderText_Solid(textFont, text.c_str(), color);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_RenderCopy(renderer, textTexture, nullptr, &rect);
};

std::string MainMenu(std::ifstream& in, SDL_Window* window, SDL_Renderer* renderer, TTF_Font* textFont) {
    SDL_Event event;
    int choice = -1;
    while (true) {
        std::cout
        << "\n Selecione um modo de jogo pra começar:"
        << "\n [1] - Tabuleiro Vazio"
        << "\n [2] - Arquivo Salvo"
        << "\n [3] - Sair"
        << "\n";

        std::vector<SDL_Rect> choiceRects = {};
        std::vector<std::string> choiceRectText = {"Arquivo Vazio", "Arquivo Salvo", "Sair"};
        for (int i = 1; i <= 3; i++) {
            choiceRects.push_back({WINDOW_WIDTH / 3, WINDOW_HEIGHT * i / 5, WINDOW_WIDTH / 3, WINDOW_HEIGHT / 5});
        }
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255 ,255);
        SDL_RenderDrawRects(renderer, choiceRects.data(), static_cast<int>(choiceRects.size()));


        for (int i = 0; i < 3; i++) {
            DrawText(renderer, choiceRects[i], choiceRectText[i], textFont, textColor);
        }

        SDL_RenderPresent(renderer);

        while (choice < 0) {
            SDL_PollEvent(&event);
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                const int mouseX = event.button.x;
                const int mouseY = event.button.y;
                SDL_Point mousePos = {mouseX, mouseY};
                for (int i = 0; i < choiceRects.size(); i++) {
                    if (SDL_PointInRect(&mousePos, &choiceRects[i])) {
                        choice = i + 1;
                        break;
                    }
                }
            }
        }

        switch (choice) {
            case 1: {
                std::vector<SDL_Rect> sizeChoiceRects = {};
                for (int i = 1; i <= 5; i++) {
                    sizeChoiceRects.push_back({WINDOW_WIDTH / 3, WINDOW_HEIGHT * i / 7, WINDOW_WIDTH / 3, WINDOW_HEIGHT / 7});
                }
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                SDL_RenderClear(renderer);
                SDL_SetRenderDrawColor(renderer, 255, 255, 255 ,255);
                SDL_RenderDrawRects(renderer, sizeChoiceRects.data(), static_cast<int>(sizeChoiceRects.size()));

                std::vector<std::string> sizeChoiceRectTexts = {"10 x 10", "25 x 25", "50 x 50", "100 x 100", "Voltar"};

                for (int i = 0; i < 5; i++) {
                    DrawText(renderer, sizeChoiceRects[i], sizeChoiceRectTexts[i], textFont, textColor);
                }

                SDL_RenderPresent(renderer);

                while (true) {
                    std::cout
                    << "\nEscolha um tamanho de tabuleiro:"
                    << "\n[1] - 10x10"
                    << "\n[2] - 25x25"
                    << "\n[3] - 50x50"
                    << "\n[4] - 100x100"
                    << "\n[5] - Voltar"
                    << "\n";
                    int sizeChoice = -1;

                    while (sizeChoice < 0) {
                        SDL_PollEvent(&event);
                        if (event.type == SDL_MOUSEBUTTONDOWN) {
                            const int mouseX = event.button.x;
                            const int mouseY = event.button.y;
                            SDL_Point mousePos = {mouseX, mouseY};
                            for (int i = 0; i < sizeChoiceRects.size(); i++) {
                                if (SDL_PointInRect(&mousePos, &sizeChoiceRects[i])) {
                                    sizeChoice = i + 1;
                                    break;
                                }
                            }
                        }
                    }
                    if (sizeChoice >= 1 && sizeChoice <= 4) {
                        const std::vector<std::string> canvasFiles = {
                            "templates/10_10_canvas.txt",
                            "templates/25_25_canvas.txt",
                            "templates/50_50_canvas.txt",
                            "templates/100_100_canvas.txt"
                        };
                        return canvasFiles[sizeChoice - 1];
                    }
                    choice = -1;
                    break;
                }
                break;
            }
            case 2: {
                std::vector<std::string> savedFiles;
                if (std::filesystem::exists(savesPath)) {
                    for (const auto& file : std::filesystem::directory_iterator(savesPath)) {
                        savedFiles.push_back(file.path().string());
                    }
                }
                if (savedFiles.empty()) {
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                    SDL_RenderClear(renderer);
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255 ,255);
                    SDL_Rect center = {WINDOW_WIDTH / 3, WINDOW_HEIGHT / 2, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2};
                    DrawText(renderer, center, "Nenhum arquivo encontrado!", textFont, textColor);
                    std::this_thread::sleep_for(static_cast<std::chrono::milliseconds>(2000));
                    SDL_RenderPresent(renderer);
                    break;
                }
                while (true) {
                    // std::cout << "Escolha um arquivo:\n";
                    // int i = 1;
                    // for (const std::string& filename : savedFiles) {
                    //     std::cout << "[" << i++ << "] - " << filename << "\n";
                    // }
                    // std::cout << "[" << i << "] - Voltar\n";

                    int fileChoice = -1;
                    int currentPage = 0;

                    std::vector<SDL_Rect> fileChoiceRects = {};
                    for (int i = 1; i <= 5; i++) {
                        fileChoiceRects.push_back({WINDOW_WIDTH / 3, WINDOW_HEIGHT * i / 7, WINDOW_WIDTH / 3, WINDOW_HEIGHT / 7});
                    }

                    SDL_Rect advancePage = {WINDOW_WIDTH * 3 / 4, WINDOW_HEIGHT / 2, WINDOW_WIDTH / 10, WINDOW_HEIGHT / 10};

                    SDL_Rect returnPage = {WINDOW_WIDTH / 4 - WINDOW_WIDTH / 10, WINDOW_HEIGHT / 2, WINDOW_WIDTH / 10, WINDOW_HEIGHT / 10};

                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                    SDL_RenderClear(renderer);
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255 ,255);
                    SDL_RenderDrawRect(renderer, &advancePage);
                    SDL_RenderDrawRect(renderer, &returnPage);
                    SDL_RenderDrawRects(renderer, fileChoiceRects.data(), static_cast<int>(fileChoiceRects.size()));
                    for (int i = 0; i < 4; i++) {
                        DrawText(renderer, fileChoiceRects[i], savedFiles[i], textFont, textColor);
                    }
                    DrawText(renderer, fileChoiceRects[4], "Voltar", textFont, textColor);
                    DrawText(renderer, advancePage, ">", textFont, textColor);
                    DrawText(renderer, returnPage, "<", textFont, textColor);

                    SDL_RenderPresent(renderer);

                    while (fileChoice < 0) {
                        SDL_PollEvent(&event);
                        if (event.type == SDL_MOUSEBUTTONDOWN) {
                            const int mouseX = event.button.x;
                            const int mouseY = event.button.y;
                            SDL_Point mousePos = {mouseX, mouseY};
                            if (SDL_PointInRect(&mousePos, &advancePage)) {
                                if ((currentPage + 1) * 4 < savedFiles.size()) {
                                    currentPage++;
                                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                                    SDL_RenderClear(renderer);
                                    SDL_SetRenderDrawColor(renderer, 255, 255, 255 ,255);
                                    SDL_RenderDrawRect(renderer, &advancePage);
                                    SDL_RenderDrawRect(renderer, &returnPage);
                                    SDL_RenderDrawRects(renderer, fileChoiceRects.data(), static_cast<int>(fileChoiceRects.size()));
                                    for (int i = 0; i < 4; i++) {
                                        DrawText(renderer, fileChoiceRects[i], savedFiles[currentPage * 4 + i], textFont, textColor);
                                    }
                                    DrawText(renderer, fileChoiceRects[4], "Voltar", textFont, textColor);
                                    DrawText(renderer, advancePage, ">", textFont, textColor);
                                    DrawText(renderer, returnPage, "<", textFont, textColor);
                                    SDL_RenderPresent(renderer);
                                    std::cout << "pag " << currentPage << "\n";
                                }
                            }
                            else if (SDL_PointInRect(&mousePos, &returnPage)) {
                                if (currentPage > 0) {
                                    currentPage--;
                                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                                    SDL_RenderClear(renderer);
                                    SDL_SetRenderDrawColor(renderer, 255, 255, 255 ,255);
                                    SDL_RenderDrawRect(renderer, &advancePage);
                                    SDL_RenderDrawRect(renderer, &returnPage);
                                    SDL_RenderDrawRects(renderer, fileChoiceRects.data(), static_cast<int>(fileChoiceRects.size()));
                                    for (int i = 0; i < 4; i++) {
                                        DrawText(renderer, fileChoiceRects[i], savedFiles[currentPage * 4 + i], textFont, textColor);
                                    }
                                    DrawText(renderer, fileChoiceRects[4], "Voltar", textFont, textColor);
                                    DrawText(renderer, advancePage, ">", textFont, textColor);
                                    DrawText(renderer, returnPage, "<", textFont, textColor);
                                    SDL_RenderPresent(renderer);
                                }
                            }
                            else {
                                for (int i = 0; i < fileChoiceRects.size(); i++) {
                                    if (SDL_PointInRect(&mousePos, &fileChoiceRects[i])) {
                                        std::cout << fileChoice + currentPage * 4;
                                        fileChoice = i;
                                        if (fileChoice + currentPage * 4 >= savedFiles.size()) {
                                            fileChoice = -1;
                                        }
                                        else {
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                    }


                    std::cout << "arquivo escolhido: "<< fileChoice;
                    if (fileChoice == 4) {
                        std::cout << "(quit)";
                    }
                    std::cout << "\n";
                    if (fileChoice <= 3) {
                        fileChoice = 4 * currentPage + fileChoice;
                        std::cout << savedFiles[fileChoice];
                        return savedFiles[fileChoice];
                    }
                    if (fileChoice == 4) {
                        choice = -1;
                        break;
                    }
                }
                break;
            }
            case 3: {
                std::cout << "Saindo...";
                return "";
            }
            default: {
                std::cout << "Operação inválida, tente novamente:\n";
                choice = -1;
                break;
            }
        }
    }
}

int main(const int argc, char ** argv) {
    int maxGenerations = 1000000;
    if (argc >= 3) maxGenerations = std::stoi(argv[2]);

    int fileCount = 0;
    std::ifstream fileCounterIn("fileCounter.txt");
    if (!fileCounterIn.is_open()) {
        throw std::runtime_error("Error opening count file!");
    }
    fileCounterIn >> fileCount;

    if (TTF_Init() == -1) {
        throw std::runtime_error("Could not initialize font system!");
    }
    TTF_Font* textFont = TTF_OpenFont(fontPath.c_str(), FONT_SIZE);

    if (!textFont) {
        throw std::runtime_error("Could not load font!");
    }

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


    std::ifstream inputFile;
    std::string inputFileName =
    MainMenu(inputFile, window, renderer, textFont);

    if (inputFileName.empty()) {
        return 0;
    }

    inputFile.open(inputFileName);
    GameBoard<char> board;
    ReadFile(inputFile, board);


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
                            board.DestroyLife(mouseY / sizeRatioY, mouseX / sizeRatioX);
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
                        if (mouseX < WINDOW_WIDTH && mouseY < WINDOW_HEIGHT && mouseX >= 0 && mouseY >= 0) {
                            board.CreateLife(mouseY / sizeRatioY, mouseX / sizeRatioX);
                        }
                    }

                    else if (mouseHeldRight) {
                        int mouseX = event.motion.x;
                        int mouseY = event.motion.y;
                        if (mouseX < WINDOW_WIDTH && mouseY < WINDOW_HEIGHT && mouseX >= 0 && mouseY >= 0) {
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
                        std::string outFilename = "saves/saved_file_" + std::to_string(fileCount) + ".txt";
                        std::ofstream outputFile(outFilename);
                        board.SaveToFile(outputFile);
                        outputFile.close();
                        std::cout << "Dados da geração " << generation << " salvos no arquivo " << outFilename << "\n";
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
        if (!paused) {
            std::this_thread::sleep_for(std::chrono_literals::operator ""ms(250));
        }
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