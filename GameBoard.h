//
// Created by john on 2025-08-25.
//

#pragma once

#include "Matrix.h"
#include <set>
#include <SDL.h>

#ifndef GAMEOFLIFE_GAMEBOARD_H
#define GAMEOFLIFE_GAMEBOARD_H

#define DEFAULT_BOARD_HEIGHT 50
#define DEFAULT_BOARD_WIDTH 50

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


#endif //GAMEOFLIFE_GAMEBOARD_H