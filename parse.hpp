// parse.hpp

#ifndef PARSE
#define PARSE

#include <vector>
#include "token.hpp"
#include "node.hpp"

// bool_expr -> ( bool_expr ) | identifier operator_equals int_literal
node* parse_bool_expr(std::vector<token>::const_iterator& it) {
    if (it->type == open_parenthesis) {
        it++; // consume (
        node* sub_be = parse_bool_expr(it);

        if (it->type != close_parenthesis) {
            std::cout << "Unpaired parentheses in boolean expression.\n";
            exit(1);
        }

        it++; // consume )   
        node* be = new node(bool_expr);
        be->components.push_back(sub_be);
        return be;
    }

    // TO DO: flatten
    if (it->type == identifier) {
        it++;

        if (it->type == operator_equals) {
            it++;

            if (it->type == int_literal) {
                it++;
                node* id = new node(node_identifier);
                node* oe = new node(node_op_equals);
                node* il = new node(node_int_literal);
                node* be = new node(bool_expr);
                be->components.push_back(id);
                be->components.push_back(oe);
                be->components.push_back(il);
                return be;
            }

            else {
                std::cout << "Error while parsing boolean expression.\nExpected an int literal after '='.\n";
                exit(1);
            }
        }

        else {
            std::cout << "Error while parsing boolean expression.\nExpected an '=' after identifier.\n";
            exit(1);
        }
    }

    else {
        std::cout << "Error while parsing boolean expression.\nExpected an ( or identifier.\n";
        exit(1);
    }
}

// where_clause -> kw_where bool_expr
node* parse_where_clause(std::vector<token>::const_iterator& it) {
    // pass kw_where
    it++;

    if (it->type == identifier || it->type == open_parenthesis) {
        node* wc = new node(where_clause);
        wc->components.push_back(parse_bool_expr(it));
        return wc;
    }
    std::cout << "Error while parsing where clause.\nExpected identifier or ( after \"where\".\n";
    exit(1);
}

// select_clause -> kw_select identifier
node* parse_select_clause(std::vector<token>::const_iterator& it) {
    it++; // consume kw_select
    if (it->type == identifier) {
        it++; // consume identifier
        node* id = new node(node_identifier);
        node* sc = new node(select_clause);
        sc->components.push_back(id);
        return sc;
    }
    std::cout << "Expected identifier after \"select\".\n";
    exit(1);
}

// from_clause -> kw_from identifier
node* parse_from_clause(std::vector<token>::const_iterator& it) {
    it++; // consume kw_from
    if (it->type == identifier) {
        it++; // consume identifier
        node* id = new node(node_identifier);
        node* fc = new node(from_clause);
        fc->components.push_back(id);
        return fc;
    }
    std::cout << "Expected identifier after \"from\".\n";
    exit(1);
}

// statement -> select_clause from_clause where_clause
node* parse(std::vector<token> tokens) {
    std::vector<token>::const_iterator it = tokens.begin();
    node* ast = new node(statement);
    ast->components.push_back(parse_select_clause(it));
    ast->components.push_back(parse_from_clause(it));
    ast->components.push_back(parse_where_clause(it));
    return ast;
}

#endif