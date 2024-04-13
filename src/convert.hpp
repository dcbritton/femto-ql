// convert.hpp

#ifndef PREPROCESSOR
#define PREPROCESSOR

#include "node.hpp"
#include "table.hpp"
#include "entry_iterator.hpp"
#include "EvaluationNode.hpp"

std::shared_ptr<EvaluationNode> convert(std::shared_ptr<node> boolExprRoot, EntryIterator& entryIteratorReference, const table& t) {

    // (bool_expr)
    if (boolExprRoot->components.size() == 1 /*only child: boolExprRoot->components[0]->type == bool_expr*/) {
        ParensNode pn;
        pn.subExpr = convert(boolExprRoot->components[0], entryIteratorReference, t);
        return std::make_shared<ParensNode>(pn);
    }

    // !(bool_expr)
    else if (boolExprRoot->components[0]->type == op_not) {
        NotNode nn;
        nn.subExpr = convert(boolExprRoot->components[1], entryIteratorReference, t);
        return std::make_shared<NotNode>(nn);
    }

    // bool_expr op_or bool_expr
    else if (boolExprRoot->components[1]->type == op_or) {
        OrNode on;
        on.lhs = convert(boolExprRoot->components[0], entryIteratorReference, t);
        on.rhs = convert(boolExprRoot->components[2], entryIteratorReference, t);
        return std::make_shared<OrNode>(on);
    }

    // bool_expr op_and bool_expr
    else if (boolExprRoot->components[1]->type == op_and) {
        AndNode an;
        an.lhs = convert(boolExprRoot->components[0], entryIteratorReference, t);
        an.rhs = convert(boolExprRoot->components[2], entryIteratorReference, t);
        return std::make_shared<AndNode>(an);
    }

    // identifier in identifier
    if (boolExprRoot->components[1]->type == kw_in) {

        auto lhsColumn = find(boolExprRoot->components[0]->value, t.columns);

        auto rhsIdentifier = split(boolExprRoot->components[2]->value);
        table rhsTable(DIRECTORY + rhsIdentifier.first + FILE_EXTENSION);
        auto rhsColumn = find(rhsIdentifier.second, rhsTable.columns);

        switch (lhsColumn->type) {
            case int_literal:
                return std::make_shared<IntInColumnNode>(lhsColumn->name, entryIteratorReference, rhsColumn->name, rhsTable);
            case float_literal:
                return std::make_shared<FloatInColumnNode>(lhsColumn->name, entryIteratorReference, rhsColumn->name, rhsTable);
            case chars_literal:
                return std::make_shared<CharsInColumnNode>(lhsColumn->name, entryIteratorReference, rhsColumn->name, rhsTable);
            case bool_literal:
                return std::make_shared<BoolInColumnNode>(lhsColumn->name, entryIteratorReference, rhsColumn->name, rhsTable);
        }
    }

    // @TODO identifier comparison any indentifier 
    else if (boolExprRoot->components[2]->type == kw_any) {
        
        element_type op = boolExprRoot->components[1]->type;
        auto lhsColumn = find(boolExprRoot->components[0]->value, t.columns);

        auto rhsIdentifier = split(boolExprRoot->components[3]->value);
        table rhsTable(DIRECTORY + rhsIdentifier.first + FILE_EXTENSION);
        auto rhsColumn = find(rhsIdentifier.second, rhsTable.columns);

        switch (lhsColumn->type) {
            case int_literal:
                return std::make_shared<IntAnyColumnComparisonNode>(lhsColumn->name, entryIteratorReference, op, rhsColumn->name, rhsTable);
            case float_literal:
                return std::make_shared<FloatAnyColumnComparisonNode>(lhsColumn->name, entryIteratorReference, op, rhsColumn->name, rhsTable);
            case chars_literal:
                return std::make_shared<CharsAnyColumnComparisonNode>(lhsColumn->name, entryIteratorReference, op, rhsColumn->name, rhsTable);
            case bool_literal:
                return std::make_shared<BoolAnyColumnComparisonNode>(lhsColumn->name, entryIteratorReference, op, rhsColumn->name, rhsTable);
        }
    }

    // @TODO identifier comparison all indentifier 
    else if (boolExprRoot->components[2]->type == kw_all) {

    }

    // identifier comparison literal/identifier
    else {
        // rhs identifier
        if (boolExprRoot->components[2]->type == identifier) {

            auto lhsColumn = find(boolExprRoot->components[0]->value, t.columns);
            auto rhsColumn = find(boolExprRoot->components[2]->value, t.columns);

            switch (lhsColumn->type) {
                case int_literal:
                    return std::make_shared<IntColumnComparisonNode>(lhsColumn->name, boolExprRoot->components[1]->type, rhsColumn->name, entryIteratorReference);
                case float_literal:
                    return std::make_shared<FloatColumnComparisonNode>(lhsColumn->name, boolExprRoot->components[1]->type, rhsColumn->name, entryIteratorReference);
                case chars_literal:
                    return std::make_shared<CharsColumnComparisonNode>(lhsColumn->name, boolExprRoot->components[1]->type, rhsColumn->name, entryIteratorReference);
                case bool_literal:
                    return std::make_shared<BoolColumnComparisonNode>(lhsColumn->name, boolExprRoot->components[1]->type, rhsColumn->name, entryIteratorReference);
            }
        }

        // rhs int literal
        else if (boolExprRoot->components[2]->type == int_literal) {
            return std::make_shared<IntLiteralComparisonNode>(boolExprRoot->components[0]->value, boolExprRoot->components[1]->type, stoi(boolExprRoot->components[2]->value), entryIteratorReference);
        }

        // rhs float literal
        else if (boolExprRoot->components[2]->type == float_literal) {
            return std::make_shared<FloatLiteralComparisonNode>(boolExprRoot->components[0]->value, boolExprRoot->components[1]->type, stof(boolExprRoot->components[2]->value), entryIteratorReference);
        }

        // rhs chars literal
        else if (boolExprRoot->components[2]->type == chars_literal) {
            return std::make_shared<CharsLiteralComparisonNode>(boolExprRoot->components[0]->value, boolExprRoot->components[1]->type, boolExprRoot->components[2]->value, entryIteratorReference);
        }

        // rhs bool literal
        else if (boolExprRoot->components[2]->type == bool_literal) {
            bool value = boolExprRoot->components[2]->value == "true";
            return std::make_shared<BoolLiteralComparisonNode>(boolExprRoot->components[0]->value, boolExprRoot->components[1]->type, value, entryIteratorReference);
        }

        // null comparison
        else if (boolExprRoot->components[2]->type == kw_null) {
            return std::make_shared<TypeAgnosticNullComparisonNode>(boolExprRoot->components[0]->value, boolExprRoot->components[1]->type, entryIteratorReference);
        }
    }
}

#endif