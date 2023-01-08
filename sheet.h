#pragma once

#include "common.h"
#include "cell.h"

#include <functional>
#include <set>
#include <map>
#include <stack>
#include <iostream>

class Cell;

class Sheet : public SheetInterface {
public:
    ~Sheet() override;

    void SetCell(Position pos, const std::string &text) override;

    const CellInterface* GetCell(Position pos) const override;

    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

private:
    enum class Color {
        WHITE,
        BLACK,
        GREY
    };

    void ResetCache(Position pos) const;

    bool CheckIfHasCycleDependencies(const std::unique_ptr<CellInterface> &new_cell, Position pos) const;

    void CheckCycleDependencies(std::unordered_map<int, std::unordered_map<int, Color>> &table_of_colors, Position pos) const;

    struct Vertex {
        std::vector<Position> edges;
        std::unique_ptr<CellInterface> formula;
    };
    Size size_ = {0, 0};
    std::unordered_map<std::string, Vertex> table_;
};