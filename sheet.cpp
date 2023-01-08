#include "sheet.h"

using namespace std::literals;

Sheet::~Sheet() {}

void Sheet::SetCell(Position pos, const std::string &text) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("incorrect pos for SetCell");
    }

    std::unique_ptr<CellInterface> new_cell = std::make_unique<Cell>(*this);
    ((Cell *) new_cell.get())->Set(text);
    if (CheckIfHasCycleDependencies(new_cell, pos)) {
        throw CircularDependencyException("Impossible to create this cell");
    }

    size_.rows = std::max(pos.row + 1, size_.rows);
    size_.cols = std::max(pos.col + 1, size_.cols);
    if (table_[pos.ToString()].formula == nullptr) {
        table_[pos.ToString()].formula = std::move(new_cell);
    }
    else {
        ResetCache(pos);
        for (const auto &i : table_[pos.ToString()].formula->GetReferencedCells()) {
            auto it = std::find(table_[i.ToString()].edges.begin(),
                                table_[i.ToString()].edges.end(), pos);
            if (it != table_[i.ToString()].edges.end()) {
                table_[i.ToString()].edges.erase(it);
            }
        }
        std::swap(new_cell, table_[pos.ToString()].formula);
    }

    for (const auto &i : table_[pos.ToString()].formula->GetReferencedCells()) {
        table_[i.ToString()].edges.push_back(pos);
        if (table_[i.ToString()].formula == nullptr) {
            table_[i.ToString()].formula = std::make_unique<Cell>(*this);
        }
    }
}

const CellInterface* Sheet::GetCell(Position pos) const {
    if (!pos.IsValid()) {
        throw InvalidPositionException("incorrect pos for GetCell");
    }
    if (!table_.count(pos.ToString())) {
        return nullptr;
    }
    else {
        return table_.at(pos.ToString()).formula.get();
    }
}

CellInterface* Sheet::GetCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("incorrect pos for GetCell");
    }
    if (!table_.count(pos.ToString())) {
        return nullptr;
    }
    else {
        return table_.at(pos.ToString()).formula.get();
    }
}

void Sheet::ClearCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("incorrect pos for ClearCell");
    }
    if (!table_.count(pos.ToString()) || table_[pos.ToString()].formula == nullptr) {
        return;
    }
    ResetCache(pos);
    table_.erase(pos.ToString());
    if (size_.cols == pos.col + 1 || size_.rows == pos.row + 1){
        size_ = {0, 0};
        for (const auto &[pos_str, vertex]: table_) {
            auto pos_pos = Position::FromString(pos_str);
            if (pos_pos.col + 1 > size_.cols) {
                size_.cols = pos_pos.col + 1;
            }
            if (pos_pos.row + 1 > size_.rows) {
                size_.rows = pos_pos.row + 1;
            }
        }
    }
    table_.erase(pos.ToString());
}

Size Sheet::GetPrintableSize() const {
    return size_;
}

void Sheet::PrintValues(std::ostream& output) const {
    for (int i = 0; i < size_.rows; ++i) {
        bool is_first = true;
        for (int s = 0; s < size_.cols; ++s) {
            if (is_first) {
                is_first = false;
            }
            else {
                output << '\t';
            }
            if (table_.count(Position{i, s}.ToString()) && table_.at(Position{i, s}.ToString()).formula != nullptr) {
                CellInterface::Value value = GetCell(Position{i, s})->GetValue();
                if (std::get_if<double>(&value)) {
                    output << std::get<double>(value);
                } else if (std::get_if<std::string>(&value)) {
                    output << std::get<std::string>(value);
                } else {
                    output << std::get<FormulaError>(value);
                }
            }
        }
        output << '\n';
    }
}
void Sheet::PrintTexts(std::ostream& output) const {
    for (int i = 0; i < size_.rows; ++i) {
        bool is_first = true;
        for (int s = 0; s < size_.cols; ++s) {
            if (is_first) {
                is_first = false;
            }
            else {
                output << '\t';
            }
            if (table_.count(Position{i, s}.ToString()) && table_.at(Position{i, s}.ToString()).formula != nullptr) {
                CellInterface::Value value = GetCell(Position{i, s})->GetText();
                if (std::get_if<double>(&value)) {
                    output << std::get<double>(value);
                } else if (std::get_if<std::string>(&value)) {
                    output << std::get<std::string>(value);
                } else {
                    output << std::get<FormulaError>(value);
                }
            }
        }
        output << '\n';
    }
}

void Sheet::ResetCache(Position pos) const {
    if (!table_.count(pos.ToString())) {
        return;
    }
    ((Cell*)table_.at(pos.ToString()).formula.get())->ResetValue();
    for (Position i : table_.at(pos.ToString()).edges) {
        ResetCache(i);
    }
}

bool Sheet::CheckIfHasCycleDependencies(const std::unique_ptr<CellInterface> &new_cell, Position pos) const {
    if (!table_.count(pos.ToString())) {
        auto ref_cells = new_cell->GetReferencedCells();
        return std::find(ref_cells.begin(), ref_cells.end(), pos) != ref_cells.end();
    }

    std::unordered_map<int, std::unordered_map<int, Color>> table_of_colors;

    table_of_colors[pos.row][pos.col] = Color::GREY;
    bool is_all_black = true;
    if (table_.count(pos.ToString())) {
        for (const auto &i: new_cell->GetReferencedCells()) {
            if (table_of_colors[i.row][i.col] == Color::WHITE) {
                CheckCycleDependencies(table_of_colors, i);
            }
            if (table_of_colors[i.row][i.col] == Color::GREY) {
                is_all_black = false;
                break;
            }
        }
    }

    return !is_all_black;
}

void Sheet::CheckCycleDependencies(std::unordered_map<int, std::unordered_map<int, Color>> &table_of_colors, Position pos) const {
    table_of_colors[pos.row][pos.col] = Color::GREY;
    bool all_black = true;
    if (!table_.count(pos.ToString())) {
        table_of_colors[pos.row][pos.col] = Color::BLACK;
        return;
    }
    for (const auto &i : table_.at(pos.ToString()).formula->GetReferencedCells()) {
        if (table_of_colors[i.row][i.col] == Color::WHITE) {
            CheckCycleDependencies(table_of_colors, i);
        }
        if (table_of_colors[i.row][i.col] == Color::GREY) {
            all_black = false;
            break;
        }
    }
    if (all_black) {
        table_of_colors[pos.row][pos.col] = Color::BLACK;
    }
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}