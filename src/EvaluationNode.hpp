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
    EntryIterator& lhsIt;
    std::string rhsColumnName;
    EntryIterator rhsIt;
    table rhsTableData;

    IntInColumnNode(const std::string& lhsColumnName, EntryIterator& lhsIt, const std::string& rhsColumnName, table rhsTableData)
        : lhsColumnName(lhsColumnName), lhsIt(lhsIt), rhsColumnName(rhsColumnName), rhsIt(rhsTableData), rhsTableData(rhsTableData) {}

    bool evaluate() override {
        rhsIt.reset();

        if (lhsIt.isNull(lhsColumnName))
            return false;

        while (rhsIt.next()) {
            if (rhsIt.isNull(rhsColumnName))
                return false;

            if (lhsIt.getInt(lhsColumnName) == rhsIt.getInt(rhsColumnName)) {
                return true;
            }
        }
        return false;
    }
};

struct FloatInColumnNode : EvaluationNode {
    std::string lhsColumnName;
    EntryIterator& lhsIt;
    std::string rhsColumnName;
    EntryIterator rhsIt;
    table rhsTableData;

    FloatInColumnNode(const std::string& lhsColumnName, EntryIterator& lhsIt, const std::string& rhsColumnName, table rhsTableData)
        : lhsColumnName(lhsColumnName), lhsIt(lhsIt), rhsColumnName(rhsColumnName), rhsIt(rhsTableData), rhsTableData(rhsTableData) {}

    bool evaluate() override {
        rhsIt.reset();

        if (lhsIt.isNull(lhsColumnName))
            return false;

        while (rhsIt.next()) {
            if (rhsIt.isNull(rhsColumnName))
                return false;

            if (lhsIt.getFloat(lhsColumnName) == rhsIt.getFloat(rhsColumnName)) {
                return true;
            }
        }
        return false;
    }
};

struct CharsInColumnNode : EvaluationNode {
    std::string lhsColumnName;
    EntryIterator& lhsIt;
    std::string rhsColumnName;
    EntryIterator rhsIt;
    table rhsTableData;

    CharsInColumnNode(const std::string& lhsColumnName, EntryIterator& lhsIt, const std::string& rhsColumnName, table rhsTableData)
        : lhsColumnName(lhsColumnName), lhsIt(lhsIt), rhsColumnName(rhsColumnName), rhsIt(rhsTableData), rhsTableData(rhsTableData) {}

    bool evaluate() override {
        rhsIt.reset();

        if (lhsIt.isNull(lhsColumnName))
            return false;

        while (rhsIt.next()) {
            if (rhsIt.isNull(rhsColumnName))
                return false;
                
            if (lhsIt.getChars(lhsColumnName) == rhsIt.getChars(rhsColumnName)) {
                return true;
            }
        }
        return false;
    }
};

struct BoolInColumnNode : EvaluationNode {
    std::string lhsColumnName;
    EntryIterator& lhsIt;
    std::string rhsColumnName;
    EntryIterator rhsIt;
    table rhsTableData;

    BoolInColumnNode(const std::string& lhsColumnName, EntryIterator& lhsIt, const std::string& rhsColumnName, table rhsTableData)
        : lhsColumnName(lhsColumnName), lhsIt(lhsIt), rhsColumnName(rhsColumnName), rhsIt(rhsTableData), rhsTableData(rhsTableData) {}

    bool evaluate() override {
        rhsIt.reset();

        if (lhsIt.isNull(lhsColumnName))
            return false;

        while (rhsIt.next()) {
            if (rhsIt.isNull(rhsColumnName))
                return false;

            if (lhsIt.getBool(lhsColumnName) == rhsIt.getBool(rhsColumnName)) {
                return true;
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