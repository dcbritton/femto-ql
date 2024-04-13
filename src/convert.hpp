// convert.hpp

#ifndef PREPROCESSOR
#define PREPROCESSOR

#include "node.hpp"
#include "TableInfo.hpp"
#include "RowIterator.hpp"
#include "EvaluationNode.hpp"

std::shared_ptr<EvaluationNode> convert(std::shared_ptr<node> boolExprRoot, RowIterator& rowItReference, const TableInfo& t) {

    // (bool_expr)
    if (boolExprRoot->components.size() == 1 /*only child: boolExprRoot->components[0]->type == bool_expr*/) {
        ParensNode pn;
        pn.subExpr = convert(boolExprRoot->components[0], rowItReference, t);
        return std::make_shared<ParensNode>(pn);
    }

    // !(bool_expr)
    else if (boolExprRoot->components[0]->type == op_not) {
        NotNode nn;
        nn.subExpr = convert(boolExprRoot->components[1], rowItReference, t);
        return std::make_shared<NotNode>(nn);
    }

    // bool_expr op_or bool_expr
    else if (boolExprRoot->components[1]->type == op_or) {
        OrNode on;
        on.lhs = convert(boolExprRoot->components[0], rowItReference, t);
        on.rhs = convert(boolExprRoot->components[2], rowItReference, t);
        return std::make_shared<OrNode>(on);
    }

    // bool_expr op_and bool_expr
    else if (boolExprRoot->components[1]->type == op_and) {
        AndNode an;
        an.lhs = convert(boolExprRoot->components[0], rowItReference, t);
        an.rhs = convert(boolExprRoot->components[2], rowItReference, t);
        return std::make_shared<AndNode>(an);
    }

    // identifier in identifier
    if (boolExprRoot->components[1]->type == kw_in) {

        auto lhsColumn = find(boolExprRoot->components[0]->value, t.columns);

        auto rhsIdentifier = split(boolExprRoot->components[2]->value);
        TableInfo rhsTable(DIRECTORY + rhsIdentifier.first + FILE_EXTENSION);
        auto rhsColumn = find(rhsIdentifier.second, rhsTable.columns);

        switch (lhsColumn->type) {
            case int_literal:
                return std::make_shared<IntInColumnNode>(lhsColumn->name, rowItReference, rhsColumn->name, rhsTable);
            case float_literal:
                return std::make_shared<FloatInColumnNode>(lhsColumn->name, rowItReference, rhsColumn->name, rhsTable);
            case chars_literal:
                return std::make_shared<CharsInColumnNode>(lhsColumn->name, rowItReference, rhsColumn->name, rhsTable);
            case bool_literal:
                return std::make_shared<BoolInColumnNode>(lhsColumn->name, rowItReference, rhsColumn->name, rhsTable);
        }
    }

    // identifier comparison any indentifier 
    else if (boolExprRoot->components[2]->type == kw_any) {
        
        element_type op = boolExprRoot->components[1]->type;
        auto lhsColumn = find(boolExprRoot->components[0]->value, t.columns);

        auto rhsIdentifier = split(boolExprRoot->components[3]->value);
        TableInfo rhsTable(DIRECTORY + rhsIdentifier.first + FILE_EXTENSION);
        auto rhsColumn = find(rhsIdentifier.second, rhsTable.columns);

        switch (lhsColumn->type) {
            case int_literal:
                return std::make_shared<IntAnyColumnComparisonNode>(lhsColumn->name, rowItReference, op, rhsColumn->name, rhsTable);
            case float_literal:
                return std::make_shared<FloatAnyColumnComparisonNode>(lhsColumn->name, rowItReference, op, rhsColumn->name, rhsTable);
            case chars_literal:
                return std::make_shared<CharsAnyColumnComparisonNode>(lhsColumn->name, rowItReference, op, rhsColumn->name, rhsTable);
            case bool_literal:
                return std::make_shared<BoolAnyColumnComparisonNode>(lhsColumn->name, rowItReference, op, rhsColumn->name, rhsTable);
        }
    }

    // @TODO identifier comparison all indentifier 
    else if (boolExprRoot->components[2]->type == kw_all) {

        element_type op = boolExprRoot->components[1]->type;
        auto lhsColumn = find(boolExprRoot->components[0]->value, t.columns);

        auto rhsIdentifier = split(boolExprRoot->components[3]->value);
        TableInfo rhsTable(DIRECTORY + rhsIdentifier.first + FILE_EXTENSION);
        auto rhsColumn = find(rhsIdentifier.second, rhsTable.columns);

        switch (lhsColumn->type) {
            case int_literal:
                return std::make_shared<IntAllColumnComparisonNode>(lhsColumn->name, rowItReference, op, rhsColumn->name, rhsTable);
            case float_literal:
                return std::make_shared<FloatAllColumnComparisonNode>(lhsColumn->name, rowItReference, op, rhsColumn->name, rhsTable);
            case chars_literal:
                return std::make_shared<CharsAllColumnComparisonNode>(lhsColumn->name, rowItReference, op, rhsColumn->name, rhsTable);
            case bool_literal:
                return std::make_shared<BoolAllColumnComparisonNode>(lhsColumn->name, rowItReference, op, rhsColumn->name, rhsTable);
        }
    }

    // identifier comparison literal/identifier
    else {
        // rhs identifier
        if (boolExprRoot->components[2]->type == identifier) {

            auto lhsColumn = find(boolExprRoot->components[0]->value, t.columns);
            auto rhsColumn = find(boolExprRoot->components[2]->value, t.columns);

            switch (lhsColumn->type) {
                case int_literal:
                    return std::make_shared<IntColumnComparisonNode>(lhsColumn->name, boolExprRoot->components[1]->type, rhsColumn->name, rowItReference);
                case float_literal:
                    return std::make_shared<FloatColumnComparisonNode>(lhsColumn->name, boolExprRoot->components[1]->type, rhsColumn->name, rowItReference);
                case chars_literal:
                    return std::make_shared<CharsColumnComparisonNode>(lhsColumn->name, boolExprRoot->components[1]->type, rhsColumn->name, rowItReference);
                case bool_literal:
                    return std::make_shared<BoolColumnComparisonNode>(lhsColumn->name, boolExprRoot->components[1]->type, rhsColumn->name, rowItReference);
            }
        }

        // rhs int literal
        else if (boolExprRoot->components[2]->type == int_literal) {
            return std::make_shared<IntLiteralComparisonNode>(boolExprRoot->components[0]->value, boolExprRoot->components[1]->type, stoi(boolExprRoot->components[2]->value), rowItReference);
        }

        // rhs float literal
        else if (boolExprRoot->components[2]->type == float_literal) {
            return std::make_shared<FloatLiteralComparisonNode>(boolExprRoot->components[0]->value, boolExprRoot->components[1]->type, stof(boolExprRoot->components[2]->value), rowItReference);
        }

        // rhs chars literal
        else if (boolExprRoot->components[2]->type == chars_literal) {
            return std::make_shared<CharsLiteralComparisonNode>(boolExprRoot->components[0]->value, boolExprRoot->components[1]->type, boolExprRoot->components[2]->value, rowItReference);
        }

        // rhs bool literal
        else if (boolExprRoot->components[2]->type == bool_literal) {
            bool value = boolExprRoot->components[2]->value == "true";
            return std::make_shared<BoolLiteralComparisonNode>(boolExprRoot->components[0]->value, boolExprRoot->components[1]->type, value, rowItReference);
        }

        // null comparison
        else if (boolExprRoot->components[2]->type == kw_null) {
            return std::make_shared<TypeAgnosticNullComparisonNode>(boolExprRoot->components[0]->value, boolExprRoot->components[1]->type, rowItReference);
        }
    }
}

#endif