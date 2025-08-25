//
// Created by john on 2025-08-25.
//
#pragma once

#include <vector>
#include <stdexcept>
#include <iostream>

#ifndef GAMEOFLIFE_MATRIX_H
#define GAMEOFLIFE_MATRIX_H

#define TRUE_VALUE '1'
#define FALSE_VALUE '0'

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

#endif //GAMEOFLIFE_MATRIX_H