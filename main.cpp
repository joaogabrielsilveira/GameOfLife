#pragma once

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <thread>
#include <SDL.h>
#include <SDL_ttf.h>
#include "Matrix.h"
#include "GameBoard.h"
#include "MenuManager.h"
#include "VideoManager.h"

constexpr SDL_Color textColor = {255, 255, 255, 255};

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
    int maxGenerations = 100000;

    int fileCount = 0;
    std::ifstream fileCounterIn("auxi/fileCounter.txt");
    if (!fileCounterIn.is_open()) {
        throw std::runtime_error("Erro ao abrir arquivo de contagem!");
    }
    fileCounterIn >> fileCount;

    if (TTF_Init() == -1) {
        throw std::runtime_error("Erro ao inicializar sistema de texto!");
    }
    TTF_Font* textFont = TTF_OpenFont(fontPath.c_str(), FONT_SIZE);

    if (!textFont) {
        throw std::runtime_error("Erro ao carregar a fonte!");
    }

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("Erro ao inicializar o sistema de vídeo.");
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Game of Life",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN
        );

    if (window == nullptr) {
        SDL_Log("Erro ao criar a janela de vídeo.");
        SDL_Quit();
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
        );

    if (renderer == nullptr) {
        SDL_Log("Erro ao criar o renderizador.");
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    GameBoard<char> board;
    auto vm = VideoManager(renderer, window, textFont);
    auto mm = MenuManager(vm, board);

    std::ifstream inputFile;
    std::string inputFileName =
    mm.MainMenu(inputFile);

    if (inputFileName.empty()) {
        vm.Terminate();
        return 0;
    }

    if (inputFileName != "0") {
        inputFile.open(inputFileName);
        ReadFile(inputFile, board);
    }

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    int generation = 0;
    bool isRunning = true;

    const int sizeRatioX = WINDOW_WIDTH / board.GetCols();
    const int sizeRatioY = WINDOW_HEIGHT / board.GetLines();

    const double srxF = static_cast<double>(WINDOW_WIDTH) / board.GetCols();
    const double sryF = static_cast<double>(WINDOW_HEIGHT) / board.GetLines();

    if (ceil(srxF) > sizeRatioX && ceil(sryF) > sizeRatioY) {
        vm.SetWindowSize(board.GetCols() * sizeRatioX + WINDOW_WIDTH / 8, board.GetLines() * sizeRatioY);
    }

    else if (ceil(sryF) > sizeRatioY) {
        vm.SetWindowSize(WINDOW_WIDTH + WINDOW_WIDTH / 8, board.GetLines() * sizeRatioY);
    }

    else if (ceil(srxF) > sizeRatioX) {
        vm.SetWindowSize(board.GetCols() * sizeRatioX + WINDOW_WIDTH / 8, WINDOW_HEIGHT);
    }

    else {
        vm.SetWindowSize(WINDOW_WIDTH + WINDOW_WIDTH / 8, WINDOW_HEIGHT);
    }

    bool paused = false;
    bool mouseHeldLeft = false;
    bool mouseHeldRight = false;
    bool mouseHeldMiddle = false;

    int circleRadius = WINDOW_WIDTH / 20;
    SDL_Point circleCenter = {WINDOW_WIDTH + WINDOW_WIDTH / 16, WINDOW_HEIGHT / 2};

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
                    else if (event.button.button == SDL_BUTTON_MIDDLE) {
                        int mouseX = event.button.x;
                        int mouseY = event.button.y;

                        if (PointInCircle({mouseX, mouseY}, circleCenter, circleRadius)) {
                            mouseHeldMiddle = true;
                        }
                    }
                    break;

                case SDL_MOUSEBUTTONUP:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        mouseHeldLeft = false;
                    }
                    if (event.button.button == SDL_BUTTON_RIGHT) {
                        mouseHeldRight = false;
                    }
                    if (event.button.button == SDL_BUTTON_MIDDLE) {
                        mouseHeldMiddle = false;
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

                    else if (mouseHeldMiddle) {
                        int mouseY = event.motion.y;

                        if (mouseY + circleRadius <= WINDOW_HEIGHT * 3 / 4 && mouseY - circleRadius >= WINDOW_HEIGHT * 1 / 4) {
                            circleCenter.y = mouseY;
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

        std::string windowTitle = "Game of Life - Generation " + std::to_string(generation);
        if (paused) windowTitle += " (Pausado)";
        vm.SetWindowTitle(windowTitle);

        board.Render(renderer, sizeRatioX, sizeRatioY);
        vm.RenderDrawCircle(circleCenter, circleRadius);
        vm.RenderPresent();

        if (!paused) {
            board.AdvanceBoardState();
            int msDelay = circleCenter.y * circleCenter.y / WINDOW_HEIGHT;
            std::this_thread::sleep_for(std::chrono::milliseconds(msDelay));
        }
    }

    vm.Terminate();

    fileCounterIn.close();
    
    std::ofstream fileCounterOut("auxi/fileCounter.txt");
    if (!fileCounterOut.is_open()) {
        throw std::runtime_error("Erro ao abrir arquivo de contagem!");
    }
    fileCounterOut << fileCount;
    fileCounterOut.close();
    inputFile.close();

    std::cout << "A simulação durou por " << generation << " gerações, e terminou com " << board.GetCurrentCount()
    << " células vivas. No total, a vida foi criada " << board.GetTotalCount() << " vezes.\n";

    return 0;
}