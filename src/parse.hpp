// parse.hpp

#ifndef PARSE
#define PARSE

#include <vector>
#include <memory>
#include "token.hpp"
#include "node.hpp"

// order_clause -> kw_order identifier asc/desc | E
std::shared_ptr<node> parse_order_clause(std::vector<token>::const_iterator& it) {

    std::vector<std::shared_ptr<node>> oc_components;

    if (it->type != kw_order) {
        std::cout << "Expecting keyword \"order\".\n";
        exit(1);
    }
    oc_components.push_back(std::make_shared<node>(it->type));
    it++; // consume kw_order

    if (it->type != identifier) {
        std::cout << "Expected identifier after \"order\".\n";
        exit(1);
    }
    oc_components.push_back(std::make_shared<node>(it->type));
    it++; // consume identifier

    if ( !(it->type == kw_asc || it->type == kw_desc) ) {
        std::cout << "Expected asc or desc after identifier.\n";
        exit(1);
    }
    oc_components.push_back(std::make_shared<node>(it->type));
    it++; // consume kw_asc/desc
        
    return std::make_shared<node>(order_clause, oc_components);
}

// bool_expr -> ( bool_expr ) | identifier op_equals int_literal
std::shared_ptr<node> parse_bool_expr(std::vector<token>::const_iterator& it) {

    std::shared_ptr<node> potential_lhs;

    // !(bool-expr)
    if (it->type == op_not) {
        it++; // consume !
        if (it->type != open_parenthesis) {
            std::cout << "Expected ( after ! in boolean expression.\n";
            exit(1);
        }
        it++; // consume (  
        
        std::shared_ptr<node> sub_be = parse_bool_expr(it);

        if (it->type != close_parenthesis) {
            std::cout << "Unpaired parentheses in boolean expression.\n";
            exit(1);
        }
        it++; // consume )  

        potential_lhs = std::make_shared<node>(bool_expr, std::vector<std::shared_ptr<node>>{std::make_shared<node>(op_not), sub_be});
    }

    // (bool_expr)
    else if (it->type == open_parenthesis) {
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
    
    std::vector<std::shared_ptr<node>> wc_components;

    if (it->type != kw_where) {
        std::cout << "Expecting keyword \"where\".\n";
        exit(1);
    }
    wc_components.push_back(std::make_shared<node>(it->type));
    it++; // consume kw_where

    wc_components.push_back(parse_bool_expr(it));

    return std::make_shared<node>(where_clause, wc_components);
}

// column_list defined by either the pattern identifier, ... identifier or single *
std::shared_ptr<node> parse_column_list(std::vector<token>::const_iterator& it) {

    std::vector<std::shared_ptr<node>> cl_components;
    // identifier list path
    if (it->type == identifier) {
        while (it->type == identifier && (it + 1)->type == comma) {
            cl_components.push_back(std::make_shared<node>(identifier));
            it += 2;
        }
        if (it->type == identifier)  {
            it++;
            cl_components.push_back(std::make_shared<node>(identifier));
        }
        else {
            std::cout << "Expected the pattern identifier, identifier... identifier in column list.\n";
            exit(1);
        }
    }
    else if (it->type == asterisk) {
        it++;
        cl_components.push_back(std::make_shared<node>(asterisk));
    }
    else {
        std::cout << "Expected column list to consist of identifiers or a *.\n";
        exit(1);
    }

    return std::make_shared<node>(column_list, cl_components);
}

// select_clause -> kw_select column_list
std::shared_ptr<node> parse_select_clause(std::vector<token>::const_iterator& it) {

    std::vector<std::shared_ptr<node>> sc_components;

    if (it->type != kw_select) {
        std::cout << "Expecting keyword \"select\" in select clause.\n";
        exit(1);
    }
    sc_components.push_back(std::make_shared<node>(it->type));
    it++; // consume kw_select

    if (it->type == kw_distinct) {
        sc_components.push_back(std::make_shared<node>(it->type));
        it++;
    }
    
    sc_components.push_back(parse_column_list(it));

    return std::make_shared<node>(select_clause, sc_components);
}

// from_clause -> kw_from identifier
std::shared_ptr<node> parse_from_clause(std::vector<token>::const_iterator& it) {
    
    std::vector<std::shared_ptr<node>> fc_components;

    if (it->type != kw_from) {
        std::cout << "Expecting keyword \"from\".\n";
        exit(1);
    }
    fc_components.push_back(std::make_shared<node>(it->type));
    it++; // consume kw_from

    if (it->type != identifier) {
        std::cout << "Expected identifier after \"from\".\n";
        exit(1);
    }
    fc_components.push_back(std::make_shared<node>(it->type));
    it++; // consume identifier

    return std::make_shared<node>(from_clause, fc_components);
}

// statement -> select_clause from_clause where_clause
std::shared_ptr<node> parse(std::vector<token> tokens) {

    std::vector<token>::const_iterator it = tokens.begin();

    // required clauses
    std::vector<std::shared_ptr<node>> st_components;
    st_components.push_back(parse_select_clause(it));
    st_components.push_back(parse_from_clause(it));

    // optional clauses
    // following token must be kw_where, kw_order, or nothing
    if (it->type == kw_where || it->type == kw_order || it == tokens.end()) {
        if (it->type == kw_where) st_components.push_back(parse_where_clause(it));
        if (it->type == kw_order) st_components.push_back(parse_order_clause(it));
        if (it == tokens.end()) return std::make_shared<node>(statement, st_components);
    }
    else {
        std::cout << "Expected a where clause, order clause, or end of statement.\n";
        exit(1);
    }

    std::cout << "Unexpected text after statement.\n";
    exit(1);
}

#endif