// EvaluationNode.hpp

#ifndef EVALUATIONNODE
#define EVALUATIONNODE

#include "Table.hpp"

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

struct IntInColumnNode : EvaluationNode {
    std::string lhsColumnName;
    Table& lhsRow;
    std::string rhsColumnName;
    Table rhsRow;

    IntInColumnNode(const std::string& lhsColumnName, Table& lhsRow, const std::string& rhsColumnName, TableInfo rhsTableData)
        : lhsColumnName(lhsColumnName), lhsRow(lhsRow), rhsColumnName(rhsColumnName), rhsRow(rhsTableData) {}

    bool evaluate() override {
        rhsRow.reset();

        if (lhsRow.isNull(lhsColumnName))
            return false;

        while (rhsRow.nextRow()) {
            if (rhsRow.isNull(rhsColumnName))
                continue;

            if (lhsRow.getInt(lhsColumnName) == rhsRow.getInt(rhsColumnName)) {
                return true;
            }
        }
        return false;
    }
};

struct FloatInColumnNode : EvaluationNode {
    std::string lhsColumnName;
    Table& lhsRow;
    std::string rhsColumnName;
    Table rhsRow;

    FloatInColumnNode(const std::string& lhsColumnName, Table& lhsRow, const std::string& rhsColumnName, TableInfo rhsTableData)
        : lhsColumnName(lhsColumnName), lhsRow(lhsRow), rhsColumnName(rhsColumnName), rhsRow(rhsTableData) {}

    bool evaluate() override {
        rhsRow.reset();

        if (lhsRow.isNull(lhsColumnName))
            return false;

        while (rhsRow.nextRow()) {
            if (rhsRow.isNull(rhsColumnName))
                continue;

            if (lhsRow.getFloat(lhsColumnName) == rhsRow.getFloat(rhsColumnName)) {
                return true;
            }
        }
        return false;
    }
};

struct CharsInColumnNode : EvaluationNode {
    std::string lhsColumnName;
    Table& lhsRow;
    std::string rhsColumnName;
    Table rhsRow;

    CharsInColumnNode(const std::string& lhsColumnName, Table& lhsRow, const std::string& rhsColumnName, TableInfo rhsTableData)
        : lhsColumnName(lhsColumnName), lhsRow(lhsRow), rhsColumnName(rhsColumnName), rhsRow(rhsTableData) {}

    bool evaluate() override {
        rhsRow.reset();

        if (lhsRow.isNull(lhsColumnName))
            return false;

        while (rhsRow.nextRow()) {
            if (rhsRow.isNull(rhsColumnName))
                continue;
                
            if (lhsRow.getChars(lhsColumnName) == rhsRow.getChars(rhsColumnName)) {
                return true;
            }
        }
        return false;
    }
};

struct BoolInColumnNode : EvaluationNode {
    std::string lhsColumnName;
    Table& lhsRow;
    std::string rhsColumnName;
    Table rhsRow;

    BoolInColumnNode(const std::string& lhsColumnName, Table& lhsRow, const std::string& rhsColumnName, TableInfo rhsTableData)
        : lhsColumnName(lhsColumnName), lhsRow(lhsRow), rhsColumnName(rhsColumnName), rhsRow(rhsTableData) {}

    bool evaluate() override {
        rhsRow.reset();

        if (lhsRow.isNull(lhsColumnName))
            return false;

        while (rhsRow.nextRow()) {
            if (rhsRow.isNull(rhsColumnName))
                continue;

            if (lhsRow.getBool(lhsColumnName) == rhsRow.getBool(rhsColumnName)) {
                return true;
            }
        }
        return false;
    }
};

struct IntAnyColumnComparisonNode : EvaluationNode {
    std::string lhsColumnName;
    Table& lhsRow;
    element_type op;
    std::string rhsColumnName;
    Table rhsRow;

    IntAnyColumnComparisonNode(const std::string& lhsColumnName, Table& lhsRow, element_type op, const std::string& rhsColumnName, TableInfo rhsTableData)
        : lhsColumnName(lhsColumnName), lhsRow(lhsRow), op(op), rhsColumnName(rhsColumnName), rhsRow(rhsTableData) {}

    bool evaluate() override {
        rhsRow.reset();

        if (lhsRow.isNull(lhsColumnName))
            return false;

        // on evaluation of each lhs row, scan the whole rhs column
        // if ever the condition is satisfied, return true
        // the condition cannot be satisfied on a null in the rhs 
        while (rhsRow.nextRow()) {
            if (rhsRow.isNull(rhsColumnName))
                continue;

            switch (op) {
                case op_equals:
                    
                    if (lhsRow.getInt(lhsColumnName) == rhsRow.getInt(rhsColumnName))
                        return true;
                    break;

                case op_not_equals:
                    if (lhsRow.getInt(lhsColumnName) != rhsRow.getInt(rhsColumnName))
                        return true;
                    break;

                case op_less_than:
                    if (lhsRow.getInt(lhsColumnName) < rhsRow.getInt(rhsColumnName))
                        return true;
                    break;

                case op_less_than_equals:
                    if (lhsRow.getInt(lhsColumnName) <= rhsRow.getInt(rhsColumnName))
                        return true;
                    break;
                
                case op_greater_than:
                    if (lhsRow.getInt(lhsColumnName) > rhsRow.getInt(rhsColumnName))
                        return true;
                    break;

                case op_greater_than_equals:
                    if (lhsRow.getInt(lhsColumnName) >= rhsRow.getInt(rhsColumnName))
                        return true;
                    break;
            }
        }
        return false;
    }
};

struct FloatAnyColumnComparisonNode : EvaluationNode {
    std::string lhsColumnName;
    Table& lhsRow;
    element_type op;
    std::string rhsColumnName;
    Table rhsRow;

    FloatAnyColumnComparisonNode(const std::string& lhsColumnName, Table& lhsRow, element_type op, const std::string& rhsColumnName, TableInfo rhsTableData)
        : lhsColumnName(lhsColumnName), lhsRow(lhsRow), op(op), rhsColumnName(rhsColumnName), rhsRow(rhsTableData) {}

    bool evaluate() override {
        rhsRow.reset();

        if (lhsRow.isNull(lhsColumnName))
            return false;

        // on evaluation of each lhs row, scan the whole rhs column
        // if ever the condition is satisfied, return true
        // the condition cannot be satisfied on a null in the rhs 
        while (rhsRow.nextRow()) {
            if (rhsRow.isNull(rhsColumnName))
                continue;

            switch (op) {
                case op_equals:
                    
                    if (lhsRow.getFloat(lhsColumnName) == rhsRow.getFloat(rhsColumnName))
                        return true;
                    break;

                case op_not_equals:
                    if (lhsRow.getFloat(lhsColumnName) != rhsRow.getFloat(rhsColumnName))
                        return true;
                    break;

                case op_less_than:
                    if (lhsRow.getFloat(lhsColumnName) < rhsRow.getFloat(rhsColumnName))
                        return true;
                    break;

                case op_less_than_equals:
                    if (lhsRow.getFloat(lhsColumnName) <= rhsRow.getFloat(rhsColumnName))
                        return true;
                    break;
                
                case op_greater_than:
                    if (lhsRow.getFloat(lhsColumnName) > rhsRow.getFloat(rhsColumnName))
                        return true;
                    break;

                case op_greater_than_equals:
                    if (lhsRow.getFloat(lhsColumnName) >= rhsRow.getFloat(rhsColumnName))
                        return true;
                    break;
            }
        }
        return false;
    }
};

struct CharsAnyColumnComparisonNode : EvaluationNode {
    std::string lhsColumnName;
    Table& lhsRow;
    element_type op;
    std::string rhsColumnName;
    Table rhsRow;

    CharsAnyColumnComparisonNode(const std::string& lhsColumnName, Table& lhsRow, element_type op, const std::string& rhsColumnName, TableInfo rhsTableData)
        : lhsColumnName(lhsColumnName), lhsRow(lhsRow), op(op), rhsColumnName(rhsColumnName), rhsRow(rhsTableData) {}

    bool evaluate() override {
        rhsRow.reset();

        if (lhsRow.isNull(lhsColumnName))
            return false;

        // on evaluation of each lhs row, scan the whole rhs column
        // if ever the condition is satisfied, return true
        // the condition cannot be satisfied on a null in the rhs 
        while (rhsRow.nextRow()) {
            if (rhsRow.isNull(rhsColumnName))
                continue;

            switch (op) {
                case op_equals:
                    
                    if (lhsRow.getChars(lhsColumnName) == rhsRow.getChars(rhsColumnName))
                        return true;
                    break;

                case op_not_equals:
                    if (lhsRow.getChars(lhsColumnName) != rhsRow.getChars(rhsColumnName))
                        return true;
                    break;

                case op_less_than:
                    if (lhsRow.getChars(lhsColumnName) < rhsRow.getChars(rhsColumnName))
                        return true;
                    break;

                case op_less_than_equals:
                    if (lhsRow.getChars(lhsColumnName) <= rhsRow.getChars(rhsColumnName))
                        return true;
                    break;
                
                case op_greater_than:
                    if (lhsRow.getChars(lhsColumnName) > rhsRow.getChars(rhsColumnName))
                        return true;
                    break;

                case op_greater_than_equals:
                    if (lhsRow.getChars(lhsColumnName) >= rhsRow.getChars(rhsColumnName))
                        return true;
                    break;
            }
        }
        return false;
    }
};

struct BoolAnyColumnComparisonNode : EvaluationNode {
    std::string lhsColumnName;
    Table& lhsRow;
    element_type op;
    std::string rhsColumnName;
    Table rhsRow;

    BoolAnyColumnComparisonNode(const std::string& lhsColumnName, Table& lhsRow, element_type op, const std::string& rhsColumnName, TableInfo rhsTableData)
        : lhsColumnName(lhsColumnName), lhsRow(lhsRow), op(op), rhsColumnName(rhsColumnName), rhsRow(rhsTableData) {}

    bool evaluate() override {
        rhsRow.reset();

        if (lhsRow.isNull(lhsColumnName))
            return false;

        // on evaluation of each lhs row, scan the whole rhs column
        // if ever the condition is satisfied, return true
        // the condition cannot be satisfied on a null in the rhs 
        while (rhsRow.nextRow()) {
            if (rhsRow.isNull(rhsColumnName))
                continue;

            switch (op) {
                case op_equals:
                    
                    if (lhsRow.getBool(lhsColumnName) == rhsRow.getBool(rhsColumnName))
                        return true;
                    break;

                case op_not_equals:
                    if (lhsRow.getBool(lhsColumnName) != rhsRow.getBool(rhsColumnName))
                        return true;
                    break;
            }
        }
        return false;
    }
};

struct IntAllColumnComparisonNode : EvaluationNode {
    std::string lhsColumnName;
    Table& lhsRow;
    element_type op;
    std::string rhsColumnName;
    Table rhsRow;

    IntAllColumnComparisonNode(const std::string& lhsColumnName, Table& lhsRow, element_type op, const std::string& rhsColumnName, TableInfo rhsTableData)
        : lhsColumnName(lhsColumnName), lhsRow(lhsRow), op(op), rhsColumnName(rhsColumnName), rhsRow(rhsTableData) {}

    bool evaluate() override {
        rhsRow.reset();

        if (lhsRow.isNull(lhsColumnName))
            return false;

        // on evaluation of each lhs row, scan the whole rhs column
        // if ever the condition is not true return false
        // on nulls in the rhs, return false. this is mysql behavior, too
        while (rhsRow.nextRow()) {
            if (rhsRow.isNull(rhsColumnName))
                return false;

            switch (op) {
                case op_equals:
                    if ( !(lhsRow.getInt(lhsColumnName) == rhsRow.getInt(rhsColumnName)) )
                        return false;
                    break;

                case op_not_equals:
                    if ( !(lhsRow.getInt(lhsColumnName) != rhsRow.getInt(rhsColumnName)) )
                        return false;
                    break;

                case op_less_than:
                    if ( !(lhsRow.getInt(lhsColumnName) < rhsRow.getInt(rhsColumnName)) )
                        return false;
                    break;

                case op_less_than_equals:
                    if ( !(lhsRow.getInt(lhsColumnName) <= rhsRow.getInt(rhsColumnName)) )
                        return false;
                    break;
                
                case op_greater_than:
                    if ( !(lhsRow.getInt(lhsColumnName) > rhsRow.getInt(rhsColumnName)) )
                        return false;
                    break;

                case op_greater_than_equals:
                    if ( !(lhsRow.getInt(lhsColumnName) >= rhsRow.getInt(rhsColumnName)) )
                        return false;
                    break;
            }
        }
        return true;
    }
};

struct FloatAllColumnComparisonNode : EvaluationNode {
    std::string lhsColumnName;
    Table& lhsRow;
    element_type op;
    std::string rhsColumnName;
    Table rhsRow;

    FloatAllColumnComparisonNode(const std::string& lhsColumnName, Table& lhsRow, element_type op, const std::string& rhsColumnName, TableInfo rhsTableData)
        : lhsColumnName(lhsColumnName), lhsRow(lhsRow), op(op), rhsColumnName(rhsColumnName), rhsRow(rhsTableData) {}

    bool evaluate() override {
        rhsRow.reset();

        if (lhsRow.isNull(lhsColumnName))
            return false;

        // on evaluation of each lhs row, scan the whole rhs column
        // if ever the condition is not true return false
        // on nulls in the rhs, return false. this is mysql behavior, too
        while (rhsRow.nextRow()) {
            if (rhsRow.isNull(rhsColumnName))
                return false;

            switch (op) {
                case op_equals:
                    if ( !(lhsRow.getFloat(lhsColumnName) == rhsRow.getFloat(rhsColumnName)) )
                        return false;
                    break;

                case op_not_equals:
                    if ( !(lhsRow.getFloat(lhsColumnName) != rhsRow.getFloat(rhsColumnName)) )
                        return false;
                    break;

                case op_less_than:
                    if ( !(lhsRow.getFloat(lhsColumnName) < rhsRow.getFloat(rhsColumnName)) )
                        return false;
                    break;

                case op_less_than_equals:
                    if ( !(lhsRow.getFloat(lhsColumnName) <= rhsRow.getFloat(rhsColumnName)) )
                        return false;
                    break;
                
                case op_greater_than:
                    if ( !(lhsRow.getFloat(lhsColumnName) > rhsRow.getFloat(rhsColumnName)) )
                        return false;
                    break;

                case op_greater_than_equals:
                    if ( !(lhsRow.getFloat(lhsColumnName) >= rhsRow.getFloat(rhsColumnName)) )
                        return false;
                    break;
            }
        }
        return true;
    }
};

struct CharsAllColumnComparisonNode : EvaluationNode {
    std::string lhsColumnName;
    Table& lhsRow;
    element_type op;
    std::string rhsColumnName;
    Table rhsRow;

    CharsAllColumnComparisonNode(const std::string& lhsColumnName, Table& lhsRow, element_type op, const std::string& rhsColumnName, TableInfo rhsTableData)
        : lhsColumnName(lhsColumnName), lhsRow(lhsRow), op(op), rhsColumnName(rhsColumnName), rhsRow(rhsTableData) {}

    bool evaluate() override {
        rhsRow.reset();

        if (lhsRow.isNull(lhsColumnName))
            return false;

        // on evaluation of each lhs row, scan the whole rhs column
        // if ever the condition is not true return false
        // on nulls in the rhs, return false. this is mysql behavior, too
        while (rhsRow.nextRow()) {
            if (rhsRow.isNull(rhsColumnName))
                return false;

            switch (op) {
                case op_equals:
                    if ( !(lhsRow.getChars(lhsColumnName) == rhsRow.getChars(rhsColumnName)) )
                        return false;
                    break;

                case op_not_equals:
                    if ( !(lhsRow.getChars(lhsColumnName) != rhsRow.getChars(rhsColumnName)) )
                        return false;
                    break;

                case op_less_than:
                    if ( !(lhsRow.getChars(lhsColumnName) < rhsRow.getChars(rhsColumnName)) )
                        return false;
                    break;

                case op_less_than_equals:
                    if ( !(lhsRow.getChars(lhsColumnName) <= rhsRow.getChars(rhsColumnName)) )
                        return false;
                    break;
                
                case op_greater_than:
                    if ( !(lhsRow.getChars(lhsColumnName) > rhsRow.getChars(rhsColumnName)) )
                        return false;
                    break;

                case op_greater_than_equals:
                    if ( !(lhsRow.getChars(lhsColumnName) >= rhsRow.getChars(rhsColumnName)) )
                        return false;
                    break;
            }
        }
        return true;
    }
};

struct BoolAllColumnComparisonNode : EvaluationNode {
    std::string lhsColumnName;
    Table& lhsRow;
    element_type op;
    std::string rhsColumnName;
    Table rhsRow;

    BoolAllColumnComparisonNode(const std::string& lhsColumnName, Table& lhsRow, element_type op, const std::string& rhsColumnName, TableInfo rhsTableData)
        : lhsColumnName(lhsColumnName), lhsRow(lhsRow), op(op), rhsColumnName(rhsColumnName), rhsRow(rhsTableData) {}

    bool evaluate() override {
        rhsRow.reset();

        if (lhsRow.isNull(lhsColumnName))
            return false;

        // on evaluation of each lhs row, scan the whole rhs column
        // if ever the condition is not true return false
        // on nulls in the rhs, return false. this is mysql behavior, too
        while (rhsRow.nextRow()) {
            if (rhsRow.isNull(rhsColumnName))
                return false;

            switch (op) {
                case op_equals:
                    if ( !(lhsRow.getBool(lhsColumnName) == rhsRow.getBool(rhsColumnName)) )
                        return false;
                    break;

                case op_not_equals:
                    if ( !(lhsRow.getBool(lhsColumnName) != rhsRow.getBool(rhsColumnName)) )
                        return false;
                    break;
            }
        }
        return true;
    }
};

struct IntColumnComparisonNode : EvaluationNode {
    std::string lhsColumnName;
    element_type op;
    std::string rhsColumnName;
    Table& row;

    IntColumnComparisonNode(const std::string& lhsColumnName, element_type op, const std::string& rhsColumnName, Table& row)
        : lhsColumnName(lhsColumnName), op(op), rhsColumnName(rhsColumnName), row(row) {}

    bool evaluate() override {

        if (row.isNull(lhsColumnName) || row.isNull(rhsColumnName))
            return false;

        switch (op) {
            case op_equals:
                return row.getInt(lhsColumnName) == row.getInt(rhsColumnName);

            case op_not_equals:
                return row.getInt(lhsColumnName) != row.getInt(rhsColumnName);

            case op_less_than:
                return row.getInt(lhsColumnName) < row.getInt(rhsColumnName);

            case op_less_than_equals:
                return row.getInt(lhsColumnName) <= row.getInt(rhsColumnName);
            
            case op_greater_than:
                return row.getInt(lhsColumnName) > row.getInt(rhsColumnName);

            case op_greater_than_equals:
                return row.getInt(lhsColumnName) >= row.getInt(rhsColumnName);
        }
    }
};

struct FloatColumnComparisonNode : EvaluationNode {
    std::string lhsColumnName;
    element_type op;
    std::string rhsColumnName;
    Table& row;

    FloatColumnComparisonNode(const std::string& lhsColumnName, element_type op, const std::string& rhsColumnName, Table& row)
        : lhsColumnName(lhsColumnName), op(op), rhsColumnName(rhsColumnName), row(row) {}

    bool evaluate() override {

        if (row.isNull(lhsColumnName) || row.isNull(rhsColumnName))
            return false;

        switch (op) {
            case op_equals:
                return row.getFloat(lhsColumnName) == row.getFloat(rhsColumnName);

            case op_not_equals:
                return row.getFloat(lhsColumnName) != row.getFloat(rhsColumnName);

            case op_less_than:
                return row.getFloat(lhsColumnName) < row.getFloat(rhsColumnName);

            case op_less_than_equals:
                return row.getFloat(lhsColumnName) <= row.getFloat(rhsColumnName);
            
            case op_greater_than:
                return row.getFloat(lhsColumnName) > row.getFloat(rhsColumnName);

            case op_greater_than_equals:
                return row.getFloat(lhsColumnName) >= row.getFloat(rhsColumnName);
        }
    }
};

struct CharsColumnComparisonNode : EvaluationNode {
    std::string lhsColumnName;
    element_type op;
    std::string rhsColumnName;
    Table& row;

    CharsColumnComparisonNode(const std::string& lhsColumnName, element_type op, const std::string& rhsColumnName, Table& row)
        : lhsColumnName(lhsColumnName), op(op), rhsColumnName(rhsColumnName), row(row) {}

    bool evaluate() override {

        if (row.isNull(lhsColumnName) || row.isNull(rhsColumnName))
            return false;

        switch (op) {
            case op_equals:
                return row.getChars(lhsColumnName) == row.getChars(rhsColumnName);

            case op_not_equals:
                return row.getChars(lhsColumnName) != row.getChars(rhsColumnName);

            case op_less_than:
                return row.getChars(lhsColumnName) < row.getChars(rhsColumnName);

            case op_less_than_equals:
                return row.getChars(lhsColumnName) <= row.getChars(rhsColumnName);
            
            case op_greater_than:
                return row.getChars(lhsColumnName) > row.getChars(rhsColumnName);

            case op_greater_than_equals:
                return row.getChars(lhsColumnName) >= row.getChars(rhsColumnName);
        }
    }
};

struct BoolColumnComparisonNode : EvaluationNode {
    std::string lhsColumnName;
    element_type op;
    std::string rhsColumnName;
    Table& row;

    BoolColumnComparisonNode(const std::string& lhsColumnName, element_type op, const std::string& rhsColumnName, Table& row)
        : lhsColumnName(lhsColumnName), op(op), rhsColumnName(rhsColumnName), row(row) {}

    bool evaluate() override {

        if (row.isNull(lhsColumnName) || row.isNull(rhsColumnName))
            return false;

        switch (op) {
            case op_equals:
                return row.getBool(lhsColumnName) == row.getBool(rhsColumnName);

            case op_not_equals:
                return row.getBool(lhsColumnName) != row.getBool(rhsColumnName);
        }
    }
};

struct IntLiteralComparisonNode : EvaluationNode {
    std::string lhsColumnName;
    element_type op;
    int literalValue;
    Table& row;

    IntLiteralComparisonNode(const std::string& lhsColumnName, element_type op, int literalValue, Table& row)
        : lhsColumnName(lhsColumnName), op(op), literalValue(literalValue), row(row) {}

    bool evaluate() override {

        if (row.isNull(lhsColumnName))
            return false;

        switch (op) {
            case op_equals:
                return row.getInt(lhsColumnName) == literalValue;

            case op_not_equals:
                return row.getInt(lhsColumnName) != literalValue;

            case op_less_than:
                return row.getInt(lhsColumnName) < literalValue;

            case op_less_than_equals:
                return row.getInt(lhsColumnName) <= literalValue;
            
            case op_greater_than:
                return row.getInt(lhsColumnName) > literalValue;

            case op_greater_than_equals:
                return row.getInt(lhsColumnName) >= literalValue;
        }
    }
};

struct FloatLiteralComparisonNode : EvaluationNode {
    std::string lhsColumnName;
    element_type op;
    float literalValue;
    Table& row;

    FloatLiteralComparisonNode(const std::string& lhsColumnName, element_type op, float literalValue, Table& row)
        : lhsColumnName(lhsColumnName), op(op), literalValue(literalValue), row(row) {}

    bool evaluate() override {

        if (row.isNull(lhsColumnName))
            return false;

        switch (op) {
            case op_equals:
                return row.getFloat(lhsColumnName) == literalValue;

            case op_not_equals:
                return row.getFloat(lhsColumnName) != literalValue;

            case op_less_than:
                return row.getFloat(lhsColumnName) < literalValue;

            case op_less_than_equals:
                return row.getFloat(lhsColumnName) <= literalValue;
            
            case op_greater_than:
                return row.getFloat(lhsColumnName) > literalValue;

            case op_greater_than_equals:
                return row.getFloat(lhsColumnName) >= literalValue;
        }
    }
};

struct CharsLiteralComparisonNode : EvaluationNode {
    std::string lhsColumnName;
    element_type op;
    std::string literalValue;
    Table& row;

    CharsLiteralComparisonNode(const std::string& lhsColumnName, element_type op, std::string& literalValue, Table& row)
        : lhsColumnName(lhsColumnName), op(op), literalValue(literalValue), row(row) {}

    bool evaluate() override {

        if (row.isNull(lhsColumnName))
            return false;

        switch (op) {
            case op_equals:
                return row.getChars(lhsColumnName) == literalValue;

            case op_not_equals:
                return row.getChars(lhsColumnName) != literalValue;

            case op_less_than:
                return row.getChars(lhsColumnName) < literalValue;

            case op_less_than_equals:
                return row.getChars(lhsColumnName) <= literalValue;
            
            case op_greater_than:
                return row.getChars(lhsColumnName) > literalValue;

            case op_greater_than_equals:
                return row.getChars(lhsColumnName) >= literalValue;
        }
    }
};

struct BoolLiteralComparisonNode : EvaluationNode {
    std::string lhsColumnName;
    element_type op;
    bool literalValue;
    Table& row;

    BoolLiteralComparisonNode(const std::string& lhsColumnName, element_type op, bool literalValue, Table& row)
        : lhsColumnName(lhsColumnName), op(op), literalValue(literalValue), row(row) {}

    bool evaluate() override {

        if (row.isNull(lhsColumnName))
            return false;

        switch (op) {
            case op_equals:
                return row.getBool(lhsColumnName) == literalValue;

            case op_not_equals:
                return row.getBool(lhsColumnName) != literalValue;

        }
    }
};

struct TypeAgnosticNullComparisonNode : EvaluationNode {
    std::string lhsColumnName;
    element_type op;
    Table& row;

    TypeAgnosticNullComparisonNode(const std::string& lhsColumnName, element_type op, Table& row)
        : lhsColumnName(lhsColumnName), op(op), row(row) {}

    bool evaluate() override {
        switch(op) {
            case op_equals:
                return row.isNull(lhsColumnName);
            case op_not_equals:
                return !row.isNull(lhsColumnName);
        }
    }
};

#endif