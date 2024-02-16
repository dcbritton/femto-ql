// parse.hpp

#ifndef PARSE
#define PARSE

#include <vector>
#include <memory>
#include "token.hpp"
#include "node.hpp"
#include "consume.hpp"

// order_clause -> kw_order identifier asc/desc | E
std::shared_ptr<node> parse_order_clause(std::vector<token>::const_iterator& it) {

    std::vector<std::shared_ptr<node>> oc_components;
    consume(kw_order, oc_components, it);
    consume(identifier, oc_components, it);

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

    // anticipate that this node may be the lefthand side of an expression with && or ||
    std::shared_ptr<node> potential_lhs;

    // !(bool-expr)
    if (it->type == op_not) {
        std::vector<std::shared_ptr<node>> lhs_components;

        lhs_components.push_back(std::make_shared<node>(it->type));
        it++; // consume !

        if (it->type != open_parenthesis) {
            std::cout << "Expected ( after ! in boolean expression.\n";
            exit(1);
        }
        it++; // consume (  
        
        lhs_components.push_back(parse_bool_expr(it));

        if (it->type != close_parenthesis) {
            std::cout << "Unpaired parentheses in boolean expression.\n";
            exit(1);
        }
        it++; // consume )  

        potential_lhs = std::make_shared<node>(bool_expr, lhs_components);
    }

    // (bool_expr)
    else if (it->type == open_parenthesis) {
        ++it; // consume (

        std::vector<std::shared_ptr<node>> lhs_components;
        lhs_components.push_back(parse_bool_expr(it));

        if (it->type != close_parenthesis) {
            std::cout << "Unpaired parentheses in boolean expression.\n";
            exit(1);
        }
        ++it; // consume )  

        potential_lhs = std::make_shared<node>(bool_expr, lhs_components);
    }

    // identifier
    else if (it->type == identifier) {

        std::vector<std::shared_ptr<node>> lhs_components;
        consume(identifier, lhs_components, it);

        // kw_in identifier
        if (it->type == kw_in) {
            consume(kw_in, lhs_components, it);
            consume(identifier, lhs_components, it);
        }
        // comparison literal
        else if (it->type >= op_equals && it->type <= op_greater_than_equals) {
            lhs_components.push_back(std::make_shared<node>(it->type));
            ++it; // consume comparison

            // literal
            if (it->type < int_literal || it->type > kw_null) {
                std::cout << "Expected an int, float, chars, or bool literal after comparison.\n";
                exit(1);
            }
            lhs_components.push_back(std::make_shared<node>(it->type));
            ++it; // consume literal
        }
        else {
            std::cout << "Expected comparison or kw_in after identifier in boolean expression.\n";
            exit(1);
        }

        potential_lhs = std::make_shared<node>(bool_expr, lhs_components);
    }
    else {
        std::cout << "Expected !, (, or identifier.\n";
        exit(1);
    }

    if (it->type == op_and || it->type == op_or) {

        std::vector<std::shared_ptr<node>> be_components;
        be_components.push_back(potential_lhs); // push left hand side
        be_components.push_back(std::make_shared<node>(it->type)); // push bool op
        ++it; // consume op_and/op_or
        be_components.push_back(parse_bool_expr(it)); // push right hand side
        
        return std::make_shared<node>(bool_expr, be_components);
    }

    return potential_lhs;
}

// where_clause -> kw_where bool_expr
std::shared_ptr<node> parse_where_clause(std::vector<token>::const_iterator& it) {
    
    std::vector<std::shared_ptr<node>> wc_components;
    consume(kw_where, wc_components, it);
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

// select_clause -> kw_select kw_distinct|ε column_list
std::shared_ptr<node> parse_select_clause(std::vector<token>::const_iterator& it) {

    std::vector<std::shared_ptr<node>> sc_components;
    consume(kw_select, sc_components, it);
    consume_optional(kw_distinct, sc_components, it);
    sc_components.push_back(parse_column_list(it));

    return std::make_shared<node>(select_clause, sc_components);
}

// from_clause -> kw_from identifier
std::shared_ptr<node> parse_from_clause(std::vector<token>::const_iterator& it) {

    std::vector<std::shared_ptr<node>> fc_components;
    consume(kw_from, fc_components, it);
    consume(identifier, fc_components, it);

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