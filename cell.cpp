#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>
#include <sstream>


// Реализуйте следующие методы
Cell::Cell(Sheet &sheet)
        : impl_(std::make_unique<EmptyImpl>())
        , sheet_(&sheet)
{
}

Cell::~Cell() = default;

void Cell::Set(const std::string &text) {
    if (text.front() == '=' && text.size() > 1) {
        impl_ = std::make_unique<FormulaImpl>(std::string{text.begin() + 1, text.end()});
    }
    else {
        impl_ = std::make_unique<TextImpl>(text);
    }
}

void Cell::Clear() {
    impl_ = std::make_unique<EmptyImpl>();
}

Cell::Value Cell::GetValue() const {
    try {
        return impl_->GetValue(*sheet_);
    }
    catch (const FormulaError &err) {
        return err;
    }
}
std::string Cell::GetText() const {
    return impl_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const {
    return impl_->GetReferencedCells();
}

bool Cell::IsReferenced() const {
    return !GetReferencedCells().empty();
}
