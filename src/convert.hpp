// convert.hpp

#ifndef PREPROCESSOR
#define PREPROCESSOR

#include "node.hpp"
#include "table.hpp"
#include <functional>
#include "entry_iterator.hpp"

struct BoolNode {
    virtual ~BoolNode() = default;
    virtual bool evaluate() = 0;
};

struct ParensNode : BoolNode {
    std::shared_ptr<BoolNode> subExpr = nullptr;

    bool evaluate() override {
        return subExpr->evaluate();
    }
};

struct NotNode : BoolNode {
    std::shared_ptr<BoolNode> subExpr = nullptr;

    bool evaluate() override {
        return !subExpr->evaluate();
    }
};

struct AndNode : BoolNode {
    std::shared_ptr<BoolNode> lhs = nullptr;
    std::shared_ptr<BoolNode> rhs = nullptr;

    bool evaluate() override {
        return lhs->evaluate() && rhs->evaluate();
    }
};

struct OrNode : BoolNode {
    std::shared_ptr<BoolNode> lhs = nullptr;
    std::shared_ptr<BoolNode> rhs = nullptr;

    bool evaluate() override {
        return lhs->evaluate() || rhs->evaluate();
    }
};

struct IntLiteralComparisonNode : BoolNode {
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

struct FloatLiteralComparisonNode : BoolNode {
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

struct CharsLiteralComparisonNode : BoolNode {
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

struct BoolLiteralComparisonNode : BoolNode {
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

struct IntColumnComparisonNode : BoolNode {
    std::string lhsColumnName;
    std::string rhsColumnName;
    EntryIterator& entry;

    bool evaluate() override {
        return entry.getInt(lhsColumnName) == entry.getInt(rhsColumnName);
    }
};

std::shared_ptr<BoolNode> convert(std::shared_ptr<node> boolExprRoot, EntryIterator& entry) {

    // (bool_expr)
    if (boolExprRoot->components.size() == 1 /*only child: boolExprRoot->components[0]->type == bool_expr*/) {
        ParensNode pn;
        pn.subExpr = convert(boolExprRoot->components[0], entry);
        return std::make_shared<ParensNode>(pn);
    }

    // !(bool_expr)
    else if (boolExprRoot->components[0]->type == op_not) {
        NotNode nn;
        nn.subExpr = convert(boolExprRoot->components[1], entry);
        return std::make_shared<NotNode>(nn);
    }

    // bool_expr op_or bool_expr
    else if (boolExprRoot->components[1]->type == op_or) {
        OrNode on;
        on.lhs = convert(boolExprRoot->components[0], entry);
        on.rhs = convert(boolExprRoot->components[2], entry);
        return std::make_shared<OrNode>(on);
    }

    // bool_expr op_and bool_expr
    else if (boolExprRoot->components[1]->type == op_and) {
        AndNode an;
        an.lhs = convert(boolExprRoot->components[0], entry);
        an.rhs = convert(boolExprRoot->components[2], entry);
        return std::make_shared<AndNode>(an);
    }

    // identifier in identifier
    if (boolExprRoot->components[1]->type == kw_in) {
        
    }

    // identifier comparison any|all indentifier 
    else if (boolExprRoot->components[2]->type == kw_any || boolExprRoot->components[2]->type == kw_all) {

    }

    // identifier comparison literal/identifier
    else {
        // rhs identifier
        if (boolExprRoot->components[2]->type == identifier) {

        }

        // rhs literal
        else if (boolExprRoot->components[2]->type == int_literal) {
            return std::make_shared<IntLiteralComparisonNode>(boolExprRoot->components[0]->value, boolExprRoot->components[1]->type, stoi(boolExprRoot->components[2]->value), entry);
        }

        // rhs literal
        else if (boolExprRoot->components[2]->type == float_literal) {

        }

        // rhs literal
        else if (boolExprRoot->components[2]->type == chars_literal) {

        }

        // rhs literal
        else if (boolExprRoot->components[2]->type == bool_literal) {

        }

        // @TODO null comparison
        else if (boolExprRoot->components[2]->type == kw_null) {

        }
    }
}

#endif