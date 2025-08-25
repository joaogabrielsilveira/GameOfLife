//
// Created by john on 2025-08-25.
//
#pragma once

#include <chrono>
#include <algorithm>
#include <SDL.h>
#include <SDL_ttf.h>
#include <filesystem>
#include <string>
#include <thread>
#include <vector>

#ifndef GAMEOFLIFE_VIDEOMANAGER_H
#define GAMEOFLIFE_VIDEOMANAGER_H

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define CIRCLE_SIDES 64
#define FONT_SIZE 48


const std::string savesPath = "saves";
const std::string fontPath = "OpenSans.ttf";

inline double DegreesToRadians(const double angle) {
    return angle / 180 * M_PI;
};

inline bool PointInCircle(const SDL_Point& p,const SDL_Point& center, const double radius) {
    const int px = p.x, py = p.y;

    // Centralizando as coordenadas do ponto no centro do círculo
    const int dx = px - center.x, dy = py - center.y;

    if (dx * dx + dy * dy < radius * radius) {
        return true;
    }
    return false;
};

class VideoManager {
public:
    VideoManager(SDL_Renderer* _renderer, SDL_Window* _window, TTF_Font* font) : _renderer(_renderer), _window(_window), _font(font) {};

    void ClearWindow() const {
        SDL_SetRenderDrawColor(_renderer, 0, 0, 0, 255);
        SDL_RenderClear(_renderer);
        SDL_SetRenderDrawColor(_renderer, 255, 255, 255, 255);
    };

    void RenderDrawPolygon(const int n, const SDL_Point& center, const double radius) const {
    if (n < 3) {
        return;
    }

    SDL_SetRenderDrawColor(_renderer, 255, 255, 255, 255);

    const double angleDiff = 360.0 / n;
    double curAngle = 0.0f;
    std::vector<SDL_Point> vertexes;

    for (int i = 0; i < n; i++) {
        const int px = center.x + static_cast<int>(round(radius * cos(DegreesToRadians(curAngle))));
        const int py = center.y + static_cast<int>(round(radius * sin(DegreesToRadians(curAngle))));
        SDL_Point p = {px, py};
        vertexes.push_back(p);
        curAngle += angleDiff;
    }

    vertexes.push_back(vertexes[0]);

        for (int i = 0; i < n; i++) {
            SDL_Point p1 = vertexes[i];
            SDL_Point p2 = vertexes[i + 1];

            const int x1 = p1.x, x2 = p2.x;
            const int y1 = p1.y, y2 = p2.y;

            SDL_RenderDrawLine(_renderer, x1, y1, x2, y2);
        }
    };

    void RenderDrawCircle (const SDL_Point center, const double radius) const {
        RenderDrawPolygon(64, center, radius);
    };

    void DrawText(const SDL_Rect& rect, const std::string& text, const SDL_Color& color = {255, 255, 255, 255}) const {
        SDL_Surface* textSurface = TTF_RenderText_Solid(_font, text.c_str(), color);
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(_renderer, textSurface);
        SDL_RenderCopy(_renderer, textTexture, nullptr, &rect);
    };

    void SetWindowTitle(const std::string& title) const {
        SDL_SetWindowTitle(_window, title.c_str());
    };

    void RenderPresent() const {
        SDL_RenderPresent(_renderer);
    };

    void Terminate() const {
        SDL_DestroyRenderer(_renderer);
        SDL_DestroyWindow(_window);
        SDL_Quit();
    };

    void DrawMenu(const int numOptions, std::vector<SDL_Rect>& boxes, const std::vector<std::string>& texts) const {
        for (int i = 1; i <= numOptions; i++) {
            SDL_Rect currentBox = {
                WINDOW_WIDTH / 3, WINDOW_HEIGHT * i / (numOptions + 2),
                WINDOW_WIDTH / 3, WINDOW_HEIGHT / (numOptions + 2)
            };
            boxes.push_back(currentBox);
        }
        ClearWindow();
        SDL_RenderDrawRects(_renderer, boxes.data(), static_cast<int>(boxes.size()));
        
        for (int i = 0; i < numOptions; i++) {
            if (i >= texts.size()) break;
            DrawText(boxes[i], texts[i]);
        }
    };

    int HandleChoice(const SDL_Point& mousePos, const std::vector<SDL_Rect>& boxes) const {
        for (int i = 0; i < boxes.size(); i++) {
            if (SDL_PointInRect(&mousePos, &boxes[i])) {
                return i;
            }
        }
        return -1;
    };
    void DrawPageChangeArrows(const SDL_Rect& advanceArrow, const SDL_Rect& returnArrow) const {
        SDL_RenderDrawRect(_renderer, &advanceArrow);
        SDL_RenderDrawRect(_renderer, &returnArrow);
        DrawText(advanceArrow, ">");
        DrawText(returnArrow, "<");
    };

    void RenderEndScreen() const {
        SetWindowTitle("Game of Life - Goodbye!");
        ClearWindow();
        constexpr SDL_Rect center = {WINDOW_WIDTH / 5, WINDOW_HEIGHT / 4, WINDOW_WIDTH * 2 / 3, WINDOW_HEIGHT / 2};
        DrawText(center, "Saindo do jogo...");
        RenderPresent();
        std::this_thread::sleep_for(static_cast<std::chrono::milliseconds>(1000));
    }

    std::string MainMenu(std::ifstream& in) const {
        SDL_Event event;
        int choice = -1;
        
        while (true) {
            SetWindowTitle("Game of Life - Main Menu");
            std::vector<SDL_Rect> choiceRects = {};
            std::vector<std::string> choiceRectText = {"Arquivo Vazio", "Arquivo Salvo", "Sair"};
        
            DrawMenu(3, choiceRects, choiceRectText);

            RenderPresent();

            while (choice < 0) {
                SDL_PollEvent(&event);
                if (event.type == SDL_QUIT) {
                    RenderEndScreen();
                    return "";
                }
                if (event.type == SDL_MOUSEBUTTONDOWN) {
                    SDL_Point mousePos = {event.button.x, event.button.y};
                    choice = HandleChoice(mousePos, choiceRects) + 1;
                }
            }

            switch (choice) {
                case 1: {
                    SetWindowTitle("Game of Life - Choose a board size");
                    std::vector<SDL_Rect> sizeChoiceRects = {};
                    std::vector<std::string> sizeChoiceRectTexts = {"10 x 10", "25 x 25", "50 x 50", "100 x 100", "Voltar"};
                    ClearWindow();
                    DrawMenu(5, sizeChoiceRects, sizeChoiceRectTexts);
                    RenderPresent();

                    while (true) {
                        int sizeChoice = -1;

                        while (sizeChoice < 0) {
                            SDL_PollEvent(&event);
                            if (event.type == SDL_QUIT) {
                                RenderEndScreen();
                                return "";
                            }
                            if (event.type == SDL_MOUSEBUTTONDOWN) {
                                SDL_Point mousePos = {event.button.x, event.button.y};
                                sizeChoice = HandleChoice(mousePos, sizeChoiceRects);
                            }
                        }
                        if (sizeChoice < 4) {
                            const std::vector<std::string> canvasFiles = {
                                "templates/10_10_canvas.txt",
                                "templates/25_25_canvas.txt",
                                "templates/50_50_canvas.txt",
                                "templates/100_100_canvas.txt"
                            };
                            return canvasFiles[sizeChoice];
                        }
                        choice = -1;
                        break;
                    }
                    break;
                }
                case 2: {
                    SetWindowTitle("Game of Life - Choose a file");
                    std::vector<std::string> savedFiles;

                    SDL_Rect advancePage = {WINDOW_WIDTH * 3 / 4, WINDOW_HEIGHT / 2, WINDOW_WIDTH / 10, WINDOW_HEIGHT / 10};

                    SDL_Rect returnPage = {WINDOW_WIDTH / 4 - WINDOW_WIDTH / 10, WINDOW_HEIGHT / 2, WINDOW_WIDTH / 10, WINDOW_HEIGHT / 10};
                    
                    if (std::filesystem::exists(savesPath)) {
                        for (const auto& file : std::filesystem::directory_iterator(savesPath)) {
                            savedFiles.push_back(file.path().string());
                        }
                    }

                    sort(savedFiles.begin(), savedFiles.end());
                    if (savedFiles.empty()) {
                        ClearWindow();
                        SDL_Rect center = {WINDOW_WIDTH / 5, WINDOW_HEIGHT / 4, WINDOW_WIDTH * 2 / 3, WINDOW_HEIGHT / 2};
                        DrawText(center, "Nenhum arquivo encontrado!");
                        RenderPresent();
                        std::this_thread::sleep_for(static_cast<std::chrono::milliseconds>(1500));
                        choice = -1;
                        break;
                    }
                    // TODO: fix back button
                    while (true) {
                        int fileChoice = -1;
                        int currentPage = 0;

                        std::vector<SDL_Rect> fileChoiceRects = {};
                        std::vector<std::string> fileChoiceRectTexts = {};
                        ClearWindow();
                        for (int i = 0; i < 4; i++) {
                            if (i >= savedFiles.size()) break;
                            fileChoiceRectTexts.push_back(savedFiles[i]);
                        }
                        fileChoiceRectTexts.push_back("Voltar");
                        
                        DrawMenu(5, fileChoiceRects, fileChoiceRectTexts);
                        DrawPageChangeArrows(advancePage, returnPage);
                        RenderPresent();

                        while (fileChoice < 0) {
                            SDL_PollEvent(&event);
                            if (event.type == SDL_QUIT) {
                                RenderEndScreen();
                                return "";
                            }
                            if (event.type == SDL_MOUSEBUTTONDOWN) {
                                SDL_Point mousePos = {event.button.x, event.button.y};
                                if (SDL_PointInRect(&mousePos, &advancePage)) {
                                    if ((currentPage + 1) * 4 < savedFiles.size()) {
                                        fileChoiceRectTexts = {};
                                        fileChoiceRects = {};
                                        currentPage++;
                                        ClearWindow();
                                        for (int i = 0; i < 4; i++) {
                                            if (currentPage * 4 + i >= savedFiles.size()) {
                                                break;
                                            }
                                            fileChoiceRectTexts.push_back(savedFiles[currentPage * 4 + i]);
                                        }
                                        DrawMenu(5, fileChoiceRects, fileChoiceRectTexts);
                                        DrawPageChangeArrows(advancePage, returnPage);
                                        std::cout << std::endl;
                                        RenderPresent();
                                    }
                                }
                                else if (SDL_PointInRect(&mousePos, &returnPage)) {
                                    if (currentPage > 0) {
                                        fileChoiceRectTexts = {};
                                        fileChoiceRects = {};
                                        currentPage--;
                                        ClearWindow();
                                        for (int i = 0; i < 4; i++) {
                                            if (currentPage * 4 + i >= savedFiles.size()) {
                                                break;
                                            }
                                            fileChoiceRectTexts.push_back(savedFiles[currentPage * 4 + i]);
                                        }
                                        DrawMenu(5, fileChoiceRects, fileChoiceRectTexts);
                                        DrawPageChangeArrows(advancePage, returnPage);
                                        RenderPresent();
                                    }
                                }
                                else {
                                    fileChoice = HandleChoice(mousePos, fileChoiceRects);
                                    std::cout << fileChoice << std::endl;
                                    std::cout << fileChoice + currentPage * 4 << std::endl;
                                    if (fileChoice + currentPage * 4 >= savedFiles.size() && fileChoice < 4) {
                                        fileChoice = -1;
                                    }
                                }
                            }
                        }

                        if (fileChoice < 4) {
                            fileChoice = 4 * currentPage + fileChoice;
                            std::cout << savedFiles[fileChoice] << std::endl;
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
                    RenderEndScreen();
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
    SDL_Renderer* _renderer;
    SDL_Window* _window;
    TTF_Font* _font;
};


#endif //GAMEOFLIFE_VIDEOMANAGER_H