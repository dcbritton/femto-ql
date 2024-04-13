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

struct IntInColumnNode : EvaluationNode {
    std::string lhsColumnName;
    EntryIterator& lhsRow;
    std::string rhsColumnName;
    EntryIterator rhsRow;

    IntInColumnNode(const std::string& lhsColumnName, EntryIterator& lhsRow, const std::string& rhsColumnName, table rhsTableData)
        : lhsColumnName(lhsColumnName), lhsRow(lhsRow), rhsColumnName(rhsColumnName), rhsRow(rhsTableData) {}

    bool evaluate() override {
        rhsRow.reset();

        if (lhsRow.isNull(lhsColumnName))
            return false;

        while (rhsRow.next()) {
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
    EntryIterator& lhsRow;
    std::string rhsColumnName;
    EntryIterator rhsRow;

    FloatInColumnNode(const std::string& lhsColumnName, EntryIterator& lhsRow, const std::string& rhsColumnName, table rhsTableData)
        : lhsColumnName(lhsColumnName), lhsRow(lhsRow), rhsColumnName(rhsColumnName), rhsRow(rhsTableData) {}

    bool evaluate() override {
        rhsRow.reset();

        if (lhsRow.isNull(lhsColumnName))
            return false;

        while (rhsRow.next()) {
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
    EntryIterator& lhsRow;
    std::string rhsColumnName;
    EntryIterator rhsRow;

    CharsInColumnNode(const std::string& lhsColumnName, EntryIterator& lhsRow, const std::string& rhsColumnName, table rhsTableData)
        : lhsColumnName(lhsColumnName), lhsRow(lhsRow), rhsColumnName(rhsColumnName), rhsRow(rhsTableData) {}

    bool evaluate() override {
        rhsRow.reset();

        if (lhsRow.isNull(lhsColumnName))
            return false;

        while (rhsRow.next()) {
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
    EntryIterator& lhsRow;
    std::string rhsColumnName;
    EntryIterator rhsRow;

    BoolInColumnNode(const std::string& lhsColumnName, EntryIterator& lhsRow, const std::string& rhsColumnName, table rhsTableData)
        : lhsColumnName(lhsColumnName), lhsRow(lhsRow), rhsColumnName(rhsColumnName), rhsRow(rhsTableData) {}

    bool evaluate() override {
        rhsRow.reset();

        if (lhsRow.isNull(lhsColumnName))
            return false;

        while (rhsRow.next()) {
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
    EntryIterator& lhsRow;
    element_type op;
    std::string rhsColumnName;
    EntryIterator rhsRow;

    IntAnyColumnComparisonNode(const std::string& lhsColumnName, EntryIterator& lhsRow, element_type op, const std::string& rhsColumnName, table rhsTableData)
        : lhsColumnName(lhsColumnName), lhsRow(lhsRow), op(op), rhsColumnName(rhsColumnName), rhsRow(rhsTableData) {}

    bool evaluate() override {
        rhsRow.reset();

        if (lhsRow.isNull(lhsColumnName))
            return false;

        // on evaluation of each lhs entry, scan the whole rhs column
        // if ever the condition is satisfied, return true
        // the condition cannot be satisfied on a null in the rhs 
        while (rhsRow.next()) {
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
    EntryIterator& lhsRow;
    element_type op;
    std::string rhsColumnName;
    EntryIterator rhsRow;

    FloatAnyColumnComparisonNode(const std::string& lhsColumnName, EntryIterator& lhsRow, element_type op, const std::string& rhsColumnName, table rhsTableData)
        : lhsColumnName(lhsColumnName), lhsRow(lhsRow), op(op), rhsColumnName(rhsColumnName), rhsRow(rhsTableData) {}

    bool evaluate() override {
        rhsRow.reset();

        if (lhsRow.isNull(lhsColumnName))
            return false;

        // on evaluation of each lhs entry, scan the whole rhs column
        // if ever the condition is satisfied, return true
        // the condition cannot be satisfied on a null in the rhs 
        while (rhsRow.next()) {
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
    EntryIterator& lhsRow;
    element_type op;
    std::string rhsColumnName;
    EntryIterator rhsRow;

    CharsAnyColumnComparisonNode(const std::string& lhsColumnName, EntryIterator& lhsRow, element_type op, const std::string& rhsColumnName, table rhsTableData)
        : lhsColumnName(lhsColumnName), lhsRow(lhsRow), op(op), rhsColumnName(rhsColumnName), rhsRow(rhsTableData) {}

    bool evaluate() override {
        rhsRow.reset();

        if (lhsRow.isNull(lhsColumnName))
            return false;

        // on evaluation of each lhs entry, scan the whole rhs column
        // if ever the condition is satisfied, return true
        // the condition cannot be satisfied on a null in the rhs 
        while (rhsRow.next()) {
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
    EntryIterator& lhsRow;
    element_type op;
    std::string rhsColumnName;
    EntryIterator rhsRow;

    BoolAnyColumnComparisonNode(const std::string& lhsColumnName, EntryIterator& lhsRow, element_type op, const std::string& rhsColumnName, table rhsTableData)
        : lhsColumnName(lhsColumnName), lhsRow(lhsRow), op(op), rhsColumnName(rhsColumnName), rhsRow(rhsTableData) {}

    bool evaluate() override {
        rhsRow.reset();

        if (lhsRow.isNull(lhsColumnName))
            return false;

        // on evaluation of each lhs entry, scan the whole rhs column
        // if ever the condition is satisfied, return true
        // the condition cannot be satisfied on a null in the rhs 
        while (rhsRow.next()) {
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

                case op_less_than:
                    if (lhsRow.getBool(lhsColumnName) < rhsRow.getBool(rhsColumnName))
                        return true;
                    break;

                case op_less_than_equals:
                    if (lhsRow.getBool(lhsColumnName) <= rhsRow.getBool(rhsColumnName))
                        return true;
                    break;
                
                case op_greater_than:
                    if (lhsRow.getBool(lhsColumnName) > rhsRow.getBool(rhsColumnName))
                        return true;
                    break;

                case op_greater_than_equals:
                    if (lhsRow.getBool(lhsColumnName) >= rhsRow.getBool(rhsColumnName))
                        return true;
                    break;
            }
        }
        return false;
    }
};

struct IntColumnComparisonNode : EvaluationNode {
    std::string lhsColumnName;
    element_type op;
    std::string rhsColumnName;
    EntryIterator& entry;

    IntColumnComparisonNode(const std::string& lhsColumnName, element_type op, const std::string& rhsColumnName, EntryIterator& entry)
        : lhsColumnName(lhsColumnName), op(op), rhsColumnName(rhsColumnName), entry(entry) {}

    bool evaluate() override {

        if (entry.isNull(lhsColumnName) || entry.isNull(rhsColumnName))
            return false;

        switch (op) {
            case op_equals:
                return entry.getInt(lhsColumnName) == entry.getInt(rhsColumnName);

            case op_not_equals:
                return entry.getInt(lhsColumnName) != entry.getInt(rhsColumnName);

            case op_less_than:
                return entry.getInt(lhsColumnName) < entry.getInt(rhsColumnName);

            case op_less_than_equals:
                return entry.getInt(lhsColumnName) <= entry.getInt(rhsColumnName);
            
            case op_greater_than:
                return entry.getInt(lhsColumnName) > entry.getInt(rhsColumnName);

            case op_greater_than_equals:
                return entry.getInt(lhsColumnName) >= entry.getInt(rhsColumnName);
        }
    }
};

struct FloatColumnComparisonNode : EvaluationNode {
    std::string lhsColumnName;
    element_type op;
    std::string rhsColumnName;
    EntryIterator& entry;

    FloatColumnComparisonNode(const std::string& lhsColumnName, element_type op, const std::string& rhsColumnName, EntryIterator& entry)
        : lhsColumnName(lhsColumnName), op(op), rhsColumnName(rhsColumnName), entry(entry) {}

    bool evaluate() override {

        if (entry.isNull(lhsColumnName) || entry.isNull(rhsColumnName))
            return false;

        switch (op) {
            case op_equals:
                return entry.getFloat(lhsColumnName) == entry.getFloat(rhsColumnName);

            case op_not_equals:
                return entry.getFloat(lhsColumnName) != entry.getFloat(rhsColumnName);

            case op_less_than:
                return entry.getFloat(lhsColumnName) < entry.getFloat(rhsColumnName);

            case op_less_than_equals:
                return entry.getFloat(lhsColumnName) <= entry.getFloat(rhsColumnName);
            
            case op_greater_than:
                return entry.getFloat(lhsColumnName) > entry.getFloat(rhsColumnName);

            case op_greater_than_equals:
                return entry.getFloat(lhsColumnName) >= entry.getFloat(rhsColumnName);
        }
    }
};

struct CharsColumnComparisonNode : EvaluationNode {
    std::string lhsColumnName;
    element_type op;
    std::string rhsColumnName;
    EntryIterator& entry;

    CharsColumnComparisonNode(const std::string& lhsColumnName, element_type op, const std::string& rhsColumnName, EntryIterator& entry)
        : lhsColumnName(lhsColumnName), op(op), rhsColumnName(rhsColumnName), entry(entry) {}

    bool evaluate() override {

        if (entry.isNull(lhsColumnName) || entry.isNull(rhsColumnName))
            return false;

        switch (op) {
            case op_equals:
                return entry.getChars(lhsColumnName) == entry.getChars(rhsColumnName);

            case op_not_equals:
                return entry.getChars(lhsColumnName) != entry.getChars(rhsColumnName);

            case op_less_than:
                return entry.getChars(lhsColumnName) < entry.getChars(rhsColumnName);

            case op_less_than_equals:
                return entry.getChars(lhsColumnName) <= entry.getChars(rhsColumnName);
            
            case op_greater_than:
                return entry.getChars(lhsColumnName) > entry.getChars(rhsColumnName);

            case op_greater_than_equals:
                return entry.getChars(lhsColumnName) >= entry.getChars(rhsColumnName);
        }
    }
};

struct BoolColumnComparisonNode : EvaluationNode {
    std::string lhsColumnName;
    element_type op;
    std::string rhsColumnName;
    EntryIterator& entry;

    BoolColumnComparisonNode(const std::string& lhsColumnName, element_type op, const std::string& rhsColumnName, EntryIterator& entry)
        : lhsColumnName(lhsColumnName), op(op), rhsColumnName(rhsColumnName), entry(entry) {}

    bool evaluate() override {

        if (entry.isNull(lhsColumnName) || entry.isNull(rhsColumnName))
            return false;

        switch (op) {
            case op_equals:
                return entry.getBool(lhsColumnName) == entry.getBool(rhsColumnName);

            case op_not_equals:
                return entry.getBool(lhsColumnName) != entry.getBool(rhsColumnName);
        }
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

        if (entry.isNull(lhsColumnName))
            return false;

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

        if (entry.isNull(lhsColumnName))
            return false;

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

        if (entry.isNull(lhsColumnName))
            return false;

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

        if (entry.isNull(lhsColumnName))
            return false;

        switch (op) {
            case op_equals:
                return entry.getBool(lhsColumnName) == literalValue;

            case op_not_equals:
                return entry.getBool(lhsColumnName) != literalValue;

        }
    }
};

struct TypeAgnosticNullComparisonNode : EvaluationNode {
    std::string lhsColumnName;
    element_type op;
    EntryIterator& entry;

    TypeAgnosticNullComparisonNode(const std::string& lhsColumnName, element_type op, EntryIterator& entry)
        : lhsColumnName(lhsColumnName), op(op), entry(entry) {}

    bool evaluate() override {
        switch(op) {
            case op_equals:
                return entry.isNull(lhsColumnName);
            case op_not_equals:
                return !entry.isNull(lhsColumnName);
        }
    }
};

#endif