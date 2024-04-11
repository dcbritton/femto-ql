// EvaluationNode.hpp

#ifndef EVALUATIONNODE
#define EVALUATIONNODE

#include "entry_iterator.hpp"

struct EvaluationNode {
    virtual ~EvaluationNode() = default;
    virtual bool evaluate() = 0;
};

struct ParensNode : EvaluationNode {
    std::shared_ptr<EvaluationNode> subExpr = nullptr;

    bool evaluate() override {
        return subExpr->evaluate();
    }
};

struct NotNode : EvaluationNode {
    std::shared_ptr<EvaluationNode> subExpr = nullptr;

    bool evaluate() override {
        return !subExpr->evaluate();
    }
};

struct AndNode : EvaluationNode {
    std::shared_ptr<EvaluationNode> lhs = nullptr;
    std::shared_ptr<EvaluationNode> rhs = nullptr;

    bool evaluate() override {
        return lhs->evaluate() && rhs->evaluate();
    }
};

struct OrNode : EvaluationNode {
    std::shared_ptr<EvaluationNode> lhs = nullptr;
    std::shared_ptr<EvaluationNode> rhs = nullptr;

    bool evaluate() override {
        return lhs->evaluate() || rhs->evaluate();
    }
};

struct IntLiteralComparisonNode : EvaluationNode {
    std::string lhsColumnName;
    element_type op;
    int literalValue;
    EntryIterator& entry;

    IntLiteralComparisonNode(const std::string& lhsColumnName, element_type op, int literalValue, EntryIterator& entry)
        : lhsColumnName(lhsColumnName), op(op), literalValue(literalValue), entry(entry) {}

    bool evaluate() override {
        switch (op) {
            case op_equals:
                return entry.getInt(lhsColumnName) == literalValue;

            case op_not_equals:
                return entry.getInt(lhsColumnName) != literalValue;

            case op_less_than:
                return entry.getInt(lhsColumnName) < literalValue;

            case op_less_than_equals:
                return entry.getInt(lhsColumnName) <= literalValue;
            
            case op_greater_than:
                return entry.getInt(lhsColumnName) > literalValue;

            case op_greater_than_equals:
                return entry.getInt(lhsColumnName) >= literalValue;
        }
    }
};

struct FloatLiteralComparisonNode : EvaluationNode {
    std::string lhsColumnName;
    element_type op;
    float literalValue;
    EntryIterator& entry;

    FloatLiteralComparisonNode(const std::string& lhsColumnName, element_type op, float literalValue, EntryIterator& entry)
        : lhsColumnName(lhsColumnName), op(op), literalValue(literalValue), entry(entry) {}

    bool evaluate() override {
        switch (op) {
            case op_equals:
                return entry.getFloat(lhsColumnName) == literalValue;

            case op_not_equals:
                return entry.getFloat(lhsColumnName) != literalValue;

            case op_less_than:
                return entry.getFloat(lhsColumnName) < literalValue;

            case op_less_than_equals:
                return entry.getFloat(lhsColumnName) <= literalValue;
            
            case op_greater_than:
                return entry.getFloat(lhsColumnName) > literalValue;

            case op_greater_than_equals:
                return entry.getFloat(lhsColumnName) >= literalValue;
        }
    }
};

struct CharsLiteralComparisonNode : EvaluationNode {
    std::string lhsColumnName;
    element_type op;
    std::string literalValue;
    EntryIterator& entry;

    CharsLiteralComparisonNode(const std::string& lhsColumnName, element_type op, std::string& literalValue, EntryIterator& entry)
        : lhsColumnName(lhsColumnName), op(op), literalValue(literalValue), entry(entry) {}

    bool evaluate() override {
        switch (op) {
            case op_equals:
                return entry.getChars(lhsColumnName) == literalValue;

            case op_not_equals:
                return entry.getChars(lhsColumnName) != literalValue;

            case op_less_than:
                return entry.getChars(lhsColumnName) < literalValue;

            case op_less_than_equals:
                return entry.getChars(lhsColumnName) <= literalValue;
            
            case op_greater_than:
                return entry.getChars(lhsColumnName) > literalValue;

            case op_greater_than_equals:
                return entry.getChars(lhsColumnName) >= literalValue;
        }
    }
};

struct BoolLiteralComparisonNode : EvaluationNode {
    std::string lhsColumnName;
    element_type op;
    bool literalValue;
    EntryIterator& entry;

    BoolLiteralComparisonNode(const std::string& lhsColumnName, element_type op, bool literalValue, EntryIterator& entry)
        : lhsColumnName(lhsColumnName), op(op), literalValue(literalValue), entry(entry) {}

    bool evaluate() override {
        switch (op) {
            case op_equals:
                return entry.getBool(lhsColumnName) == literalValue;

            case op_not_equals:
                return entry.getBool(lhsColumnName) != literalValue;
        }
    }
};

struct IntColumnComparisonNode : EvaluationNode {
    std::string lhsColumnName;
    std::string rhsColumnName;
    EntryIterator& entry;

    bool evaluate() override {
        return entry.getInt(lhsColumnName) == entry.getInt(rhsColumnName);
    }
};

#endif