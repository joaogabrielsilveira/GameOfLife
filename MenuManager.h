#ifndef GAMEOFLIFE_MENUMANAGER_H
#define GAMEOFLIFE_MENUMANAGER_H

#pragma once

#include <algorithm>
#include "VideoManager.h"
#include "GameBoard.h"


class MenuManager {
public:
    MenuManager(VideoManager& vm, GameBoard<char>& tb) : _vm(vm), _tb(tb) {};

    [[nodiscard]] static int HandleChoice(const SDL_Point& mousePos, const std::vector<SDL_Rect>& boxes) {
        for (int i = 0; i < boxes.size(); i++) {
            if (SDL_PointInRect(&mousePos, &boxes[i])) {
                return i;
            }
        }
        return -1;
    };

    std::string MainMenu(std::ifstream& in) const {
        if (!std::filesystem::exists(savesPath)) {
            while (!std::filesystem::create_directory(savesPath));
        }
        SDL_Event event;
        int choice = -1;

        while (true) {
            _vm.SetWindowTitle("Game of Life - Main Menu");
            std::vector<SDL_Rect> choiceRects = {};
            std::vector<std::string> choiceRectText = {"Arquivo Vazio", "Arquivo Salvo", "Sair"};

            _vm.DrawMenu(3, choiceRects, choiceRectText);

            _vm.RenderPresent();

            while (choice < 0) {
                SDL_PollEvent(&event);
                if (event.type == SDL_QUIT) {
                    _vm.RenderEndScreen();
                    return "";
                }
                if (event.type == SDL_MOUSEBUTTONDOWN) {
                    SDL_Point mousePos = {event.button.x, event.button.y};
                    choice = HandleChoice(mousePos, choiceRects) + 1;
                }
            }

            switch (choice) {
                case 1: {
                    _vm.SetWindowTitle("Game of Life - Choose a board size");
                    std::vector<SDL_Rect> sizeChoiceRects = {};
                    std::vector<std::string> sizeChoiceRectTexts = {"10 x 10", "25 x 25", "50 x 50", "100 x 100", "Voltar"};
                    _vm.ClearWindow();
                    _vm.DrawMenu(5, sizeChoiceRects, sizeChoiceRectTexts);
                    _vm.RenderPresent();

                    while (true) {
                        int sizeChoice = -1;

                        while (sizeChoice < 0) {
                            SDL_PollEvent(&event);
                            if (event.type == SDL_QUIT) {
                                _vm.RenderEndScreen();
                                return "";
                            }
                            if (event.type == SDL_MOUSEBUTTONDOWN) {
                                SDL_Point mousePos = {event.button.x, event.button.y};
                                sizeChoice = HandleChoice(mousePos, sizeChoiceRects);
                            }
                        }
                        if (sizeChoice < 4) {
                            const std::vector<int> canvasSizes = {10, 25, 50, 100};
                            _tb.InitEmpty(canvasSizes[sizeChoice], canvasSizes[sizeChoice]);
                            return "0";
                        }
                        choice = -1;
                        break;
                    }
                    break;
                }
                case 2: {
                    _vm.SetWindowTitle("Game of Life - Choose a file");
                    std::vector<std::string> savedFiles;

                    SDL_Rect advancePage = {WINDOW_WIDTH * 3 / 4, WINDOW_HEIGHT / 2, WINDOW_WIDTH / 10, WINDOW_HEIGHT / 10};

                    SDL_Rect returnPage = {WINDOW_WIDTH / 4 - WINDOW_WIDTH / 10, WINDOW_HEIGHT / 2, WINDOW_WIDTH / 10, WINDOW_HEIGHT / 10};


                    for (const auto& file : std::filesystem::directory_iterator(savesPath)) {
                        savedFiles.push_back(file.path().string().substr(6));
                    }

                    if (savedFiles.empty()) {
                        _vm.ClearWindow();
                        SDL_Rect center = {WINDOW_WIDTH / 5, WINDOW_HEIGHT / 4, WINDOW_WIDTH * 2 / 3, WINDOW_HEIGHT / 2};
                        _vm.DrawText(center, "Nenhum arquivo!");
                        _vm.RenderPresent();
                        std::this_thread::sleep_for(static_cast<std::chrono::milliseconds>(1500));
                        choice = -1;
                        break;
                    }

                    sort(savedFiles.begin(), savedFiles.end());

                    while (true) {
                        int fileChoice = -1;
                        int currentPage = 0;

                        std::vector<SDL_Rect> fileChoiceRects = {};
                        std::vector<std::string> fileChoiceRectTexts = {};
                        _vm.ClearWindow();
                        for (int i = 0; i < 4; i++) {
                            if (i >= savedFiles.size()) break;
                            fileChoiceRectTexts.push_back(savedFiles[i]);
                        }

                        _vm.DrawMenu(5, fileChoiceRects, fileChoiceRectTexts, true);
                        _vm.DrawPageChangeArrows(advancePage, returnPage);
                        _vm.RenderPresent();

                        while (fileChoice < 0) {
                            SDL_PollEvent(&event);
                            if (event.type == SDL_QUIT) {
                                _vm.RenderEndScreen();
                                return "";
                            }
                            if (event.type == SDL_MOUSEBUTTONDOWN) {
                                SDL_Point mousePos = {event.button.x, event.button.y};
                                if (SDL_PointInRect(&mousePos, &advancePage)) {
                                    if ((currentPage + 1) * 4 < savedFiles.size()) {
                                        fileChoiceRectTexts = {};
                                        fileChoiceRects = {};
                                        currentPage++;
                                        _vm.ClearWindow();
                                        for (int i = 0; i < 4; i++) {
                                            if (currentPage * 4 + i >= savedFiles.size()) {
                                                break;
                                            }
                                            fileChoiceRectTexts.push_back(savedFiles[currentPage * 4 + i]);
                                        }
                                        _vm.DrawMenu(5, fileChoiceRects, fileChoiceRectTexts, true);
                                        _vm.DrawPageChangeArrows(advancePage, returnPage);
                                        _vm.RenderPresent();
                                    }
                                }
                                else if (SDL_PointInRect(&mousePos, &returnPage)) {
                                    if (currentPage > 0) {
                                        fileChoiceRectTexts = {};
                                        fileChoiceRects = {};
                                        currentPage--;
                                        _vm.ClearWindow();
                                        for (int i = 0; i < 4; i++) {
                                            if (currentPage * 4 + i >= savedFiles.size()) {
                                                break;
                                            }
                                            fileChoiceRectTexts.push_back(savedFiles[currentPage * 4 + i]);
                                        }
                                        _vm.DrawMenu(5, fileChoiceRects, fileChoiceRectTexts, true);
                                        _vm.DrawPageChangeArrows(advancePage, returnPage);
                                        _vm.RenderPresent();
                                    }
                                }
                                else {
                                    fileChoice = HandleChoice(mousePos, fileChoiceRects);
                                    if (fileChoice + currentPage * 4 >= savedFiles.size() && fileChoice < 4) {
                                        fileChoice = -1;
                                    }
                                }
                            }
                        }

                        if (fileChoice < 4) {
                            fileChoice = 4 * currentPage + fileChoice;
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
                    _vm.RenderEndScreen();
                    return "";
                }
                default: {
                    choice = -1;
                    break;
                }
            }
        }
    }
private:
    VideoManager& _vm;
    GameBoard<char>& _tb;

};


#endif //GAMEOFLIFE_MENUMANAGER_H