// convert.hpp

#ifndef PREPROCESSOR
#define PREPROCESSOR

#include "node.hpp"
#include "table.hpp"
#include "entry_iterator.hpp"
#include "EvaluationNode.hpp"

std::shared_ptr<EvaluationNode> convert(std::shared_ptr<node> boolExprRoot, EntryIterator& entry) {

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
            return std::make_shared<FloatLiteralComparisonNode>(boolExprRoot->components[0]->value, boolExprRoot->components[1]->type, stof(boolExprRoot->components[2]->value), entry);
        }

        // rhs literal
        else if (boolExprRoot->components[2]->type == chars_literal) {
            return std::make_shared<CharsLiteralComparisonNode>(boolExprRoot->components[0]->value, boolExprRoot->components[1]->type, boolExprRoot->components[2]->value, entry);
        }

        // rhs literal
        else if (boolExprRoot->components[2]->type == bool_literal) {
            bool value = boolExprRoot->components[2]->value == "true";
            return std::make_shared<BoolLiteralComparisonNode>(boolExprRoot->components[0]->value, boolExprRoot->components[1]->type, value, entry);
        }

        // @TODO null comparison, with type independent null comparison node?
        else if (boolExprRoot->components[2]->type == kw_null) {

        }
    }
}

#endif