// parse.hpp

#ifndef PARSE
#define PARSE

#include <vector>
#include <memory>
#include "token.hpp"
#include "node.hpp"

// bool_expr -> ( bool_expr ) | identifier op_equals int_literal
std::shared_ptr<node> parse_bool_expr(std::vector<token>::const_iterator& it) {

    std::shared_ptr<node> potential_lhs;

    // (bool_expr)
    if (it->type == open_parenthesis) {
        it++; // consume (
        std::shared_ptr<node> sub_be = parse_bool_expr(it);

        if (it->type != close_parenthesis) {
            std::cout << "Unpaired parentheses in boolean expression.\n";
            exit(1);
        }
        it++; // consume )  

        potential_lhs = std::make_shared<node>(bool_expr, std::vector<std::shared_ptr<node>>{sub_be});
    }

    // identifier comparison int_literal
    else if (it->type == identifier) {
        it++;

        std::shared_ptr<node> op_comparison;
        if (it->type >= op_equals && it->type <= op_greater_than_equals) {
            op_comparison = std::make_shared<node>(it->type);
            it++;
        }
        else {
            std::cout << "Error while parsing boolean expression.\nExpected a comparison after identifier.\n";
            exit(1);
        }

        if (it->type == int_literal) it++;
        else {
            std::cout << "Error while parsing boolean expression.\nExpected an int literal after '=='.\n";
            exit(1);
        }
    
        std::vector<std::shared_ptr<node>> temp = { std::make_shared<node>(identifier), 
                                                   op_comparison,
                                                   std::make_shared<node>(int_literal) };
        potential_lhs = std::make_shared<node>(bool_expr, temp);
    }
    else {
        std::cout << "Error while parsing boolean expression.\nExpected an ( or identifier.\n";
        exit(1);
    }

    if (it->type == op_and) {
        it++; // consume op_and
        std::shared_ptr<node> rhs = parse_bool_expr(it);
        std::vector<std::shared_ptr<node>> t = {potential_lhs, std::make_shared<node>(op_and), rhs};
        return std::make_shared<node>(bool_expr, t);
    }

    if (it->type == op_or) {
        it++; // consume op_or
        std::shared_ptr<node> rhs = parse_bool_expr(it);
        std::vector<std::shared_ptr<node>> t = {potential_lhs, std::make_shared<node>(op_or), rhs};
        return std::make_shared<node>(bool_expr, t);
    }

    return potential_lhs;
}

// where_clause -> kw_where bool_expr
std::shared_ptr<node> parse_where_clause(std::vector<token>::const_iterator& it) {
    // pass kw_where
    it++;

    if (it->type == identifier || it->type == open_parenthesis) {
        return std::make_shared<node>(where_clause, std::vector<std::shared_ptr<node>>{parse_bool_expr(it)});
    }
    std::cout << "Error while parsing where clause.\nExpected identifier or ( after \"where\".\n";
    exit(1);
}

// select_clause -> kw_select identifier
std::shared_ptr<node> parse_select_clause(std::vector<token>::const_iterator& it) {
    it++; // consume kw_select
    if (it->type == identifier) {
        it++; // consume identifier
        std::vector<std::shared_ptr<node>> temp = {std::make_shared<node>(identifier)};
        return std::make_shared<node>(select_clause, temp);
    }
    std::cout << "Expected identifier after \"select\".\n";
    exit(1);
}

// from_clause -> kw_from identifier
std::shared_ptr<node> parse_from_clause(std::vector<token>::const_iterator& it) {
    it++; // consume kw_from
    if (it->type == identifier) {
        it++; // consume identifier
        std::vector<std::shared_ptr<node>> temp = {std::make_shared<node>(identifier)};
        return std::make_shared<node>(from_clause, temp);
    }
    std::cout << "Expected identifier after \"from\".\n";
    exit(1);
}

// statement -> select_clause from_clause where_clause
std::shared_ptr<node> parse(std::vector<token> tokens) {
    std::vector<token>::const_iterator it = tokens.begin();
    std::shared_ptr<node> sc = parse_select_clause(it);
    std::shared_ptr<node> fc = parse_from_clause(it);
    std::shared_ptr<node> wc = parse_where_clause(it);
    return std::make_shared<node>(statement, std::vector<std::shared_ptr<node>>{sc, fc, wc});
}

#endif