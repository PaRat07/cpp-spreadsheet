#pragma once

#include "common.h"
#include "formula.h"
#include "sheet.h"

#include <functional>
#include <unordered_set>

class Sheet;

class Cell : public CellInterface {
public:
    Cell(Sheet &sheet);

    ~Cell();

    void Set(const std::string &text);

    void Clear();

    void ResetValue() const {
        impl_->ResetValue();
    }

    Value GetValue() const override;

    std::string GetText() const override;

    std::vector<Position> GetReferencedCells() const override;

    bool IsReferenced() const;

private:
    class Impl {
    public:
        virtual Value GetValue(const SheetInterface &sheet) const = 0;
        virtual std::string GetText() const = 0;
        virtual std::vector<Position> GetReferencedCells() const = 0;
        virtual ~Impl() = default;
        virtual void ResetValue() const = 0;

    };
    class EmptyImpl : public Impl {
    public:
        Value GetValue(const SheetInterface &) const override {
            return "";
        }
        std::string GetText() const override {
            return "";
        }
        std::vector<Position> GetReferencedCells() const override {
            return {};
        }
        void ResetValue() const override {}
        ~EmptyImpl() override = default;
    };
    class TextImpl : public Impl {
        std::string text_;
    public:
        TextImpl(std::string text)
                : text_(std::move(text))
        {
        }

        Value GetValue(const SheetInterface &) const override {
            std::string ans{text_};
            if (ans.front() == '\'') {
                ans.erase(ans.begin());
            }
            return ans;
        }
        std::string GetText() const override {
            return text_;
        }
        std::vector<Position> GetReferencedCells() const override {
            return {};
        }
        void ResetValue() const override {}
        ~TextImpl() override = default;
    };
    class FormulaImpl : public Impl {
        std::unique_ptr<FormulaInterface> formula_;
        mutable std::unique_ptr<Value> val_ = nullptr;
    public:
        FormulaImpl(std::string text)
                : formula_(ParseFormula(std::move(text)))
        {
        }

        Value GetValue(const SheetInterface &sheet) const override {
            if (val_ == nullptr) {
                auto buf = formula_->Evaluate(sheet);
                if (std::get_if<double>(&buf)) {
                    val_ = std::make_unique<Value>(std::get<double>(buf));
                }
                else {
                    val_ = std::make_unique<Value>(std::get<FormulaError>(buf));
                }
            }
            return *val_;
        }
        std::string GetText() const override {
            return "=" + formula_->GetExpression();
        }
        std::vector<Position> GetReferencedCells() const override {
            return formula_->GetReferencedCells();
        }
        void ResetValue() const override {
            val_.reset();
        }
        ~FormulaImpl() override = default;
    };

    std::unique_ptr<Impl> impl_;
    Sheet *sheet_;
};