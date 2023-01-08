#include "formula.h"
#include "FormulaAST.h"

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << "#DIV/0!";
}

namespace {
    class Formula : public FormulaInterface {
    public:
        // Реализуйте следующие методы:
        explicit Formula(const std::string &expression) {
            try {
                ast_ = new FormulaAST(ParseFormulaAST(expression));
            }
            catch (...) {
                throw FormulaException("incorrect formula");
            }
        }
        Value Evaluate(const SheetInterface& sheet) const override {
            try {
                return ast_->Execute(sheet);
            }
            catch(const FormulaError &err) {
                return err;
            }
        }
        std::string GetExpression() const override {
            std::ostringstream buf;
            ast_->PrintFormula(buf);
            return buf.str();
        }

        std::vector<Position> GetReferencedCells() const override {
            std::vector<Position> ans{ast_->GetCells().begin(), ast_->GetCells().end()};
            ans.erase(std::unique(ans.begin(), ans.end()), ans.end());
            return ans;
        }

    private:
        FormulaAST *ast_;
    };
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(const std::string &expression) {
    return std::make_unique<Formula>(expression);
}