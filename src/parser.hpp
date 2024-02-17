// parser.hpp

#ifndef PARSER
#define PARSER

#include <vector>
#include <memory>
#include "token.hpp"
#include "node.hpp"
#include "consume.hpp"

class Parser {
private:
    std::vector<token> tokens = {};
    std::vector<token>::const_iterator it;
    element_type current_non_terminal;
public:

    Parser(std::vector<token> token_stream) 
        : tokens(token_stream), it(tokens.begin()) {};

    // order_clause -> kw_order identifier asc/desc | E
    std::shared_ptr<node> parse_order_clause() {

        std::vector<std::shared_ptr<node>> oc_components;
        consume(kw_order, oc_components);
        consume(identifier, oc_components);

        if (it->type != kw_asc && it->type != kw_desc)  {
            std::cout << "Expected asc or desc after identifier.\n";
            exit(1);
        }
        oc_components.push_back(std::make_shared<node>(it->type));
        it++; // consume kw_asc/desc
            
        return std::make_shared<node>(order_clause, oc_components);
    }

    // bool_expr -> ( bool_expr ) | identifier op_equals int_literal
    std::shared_ptr<node> parse_bool_expr() {

        // anticipate that this node may be the lefthand side of an expression with && or ||
        std::shared_ptr<node> potential_lhs;

        // !(bool-expr)
        if (it->type == op_not) {

            std::vector<std::shared_ptr<node>> lhs_components;
            consume(op_not, lhs_components);
            discard(open_parenthesis);
            lhs_components.push_back(parse_bool_expr());
            discard(close_parenthesis);

            potential_lhs = std::make_shared<node>(bool_expr, lhs_components);
        }

        // (bool_expr)
        else if (it->type == open_parenthesis) {

            std::vector<std::shared_ptr<node>> lhs_components;
            discard(open_parenthesis);
            lhs_components.push_back(parse_bool_expr());
            discard(close_parenthesis);

            potential_lhs = std::make_shared<node>(bool_expr, lhs_components);
        }

        // identifier
        else if (it->type == identifier) {

            std::vector<std::shared_ptr<node>> lhs_components;
            consume(identifier, lhs_components);

            // kw_in identifier
            if (it->type == kw_in) {
                consume(kw_in, lhs_components);
                consume(identifier, lhs_components);
            }

            // comparison
            else if (it->type >= op_equals && it->type <= op_greater_than_equals) {
                lhs_components.push_back(std::make_shared<node>(it->type));
                ++it; // consume comparison

                // literal (incl. null)
                if (it->type >= int_literal && it->type <= kw_null) {
                    lhs_components.push_back(std::make_shared<node>(it->type));
                    ++it; // consume literal
                }

                // any|all
                else if (it->type == kw_any || it->type == kw_all) {
                    lhs_components.push_back(std::make_shared<node>(it->type));
                    ++it; // consume kw_any/kw_all
                    consume(identifier, lhs_components);
                }
                else {
                    std::cout << "Expected keyword any/all/null or int/float/chars/bool literal after comparison.\n";
                    exit(1);
                }
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

        // && or ||
        if (it->type == op_and || it->type == op_or) {

            std::vector<std::shared_ptr<node>> be_components;
            be_components.push_back(potential_lhs); // push left hand side
            be_components.push_back(std::make_shared<node>(it->type)); // push bool op
            ++it; // consume op_and/op_or
            be_components.push_back(parse_bool_expr()); // push right hand side
            
            return std::make_shared<node>(bool_expr, be_components);
        }

        // no && or ||
        return potential_lhs;
    }

    // where_clause -> kw_where bool_expr
    std::shared_ptr<node> parse_where_clause() {
        
        std::vector<std::shared_ptr<node>> wc_components;
        consume(kw_where, wc_components);
        wc_components.push_back(parse_bool_expr());

        return std::make_shared<node>(where_clause, wc_components);
    }

    // column_list -> identifier, ... identifier 
    //              | *
    std::shared_ptr<node> parse_column_list() {

        std::vector<std::shared_ptr<node>> cl_components;
        // identifier list path
        if (it->type == identifier) {
            while (it->type == identifier && (it + 1)->type == comma) {
                consume(identifier, cl_components);
                consume(comma, cl_components);
            }
            consume(identifier, cl_components);
        }
        // *
        else if (it->type == asterisk)
            consume(asterisk, cl_components);
        else {
            std::cout << "Expected column list to consist of identifiers or a *.\n";
            exit(1);
        }

        return std::make_shared<node>(column_list, cl_components);
    }

    // select_clause -> kw_select kw_distinct|ε column_list
    std::shared_ptr<node> parse_select_clause() {

        std::vector<std::shared_ptr<node>> sc_components;
        consume(kw_select, sc_components);
        consume_optional(kw_distinct, sc_components);
        sc_components.push_back(parse_column_list());

        return std::make_shared<node>(select_clause, sc_components);
    }

    // from_clause -> kw_from identifier
    std::shared_ptr<node> parse_from_clause() {

        std::vector<std::shared_ptr<node>> fc_components;
        consume(kw_from, fc_components);
        consume(identifier, fc_components);

        return std::make_shared<node>(from_clause, fc_components);
    }

    //select_clause from_clause where_clause|ε order_clause|ε
    std::shared_ptr<node> parse_selection() {

        // required clauses
        std::vector<std::shared_ptr<node>> st_components;
        st_components.push_back(parse_select_clause());
        st_components.push_back(parse_from_clause());

        // optional clauses
        // following token must be kw_where, kw_order, or nothing
        if (it->type == kw_where || it->type == kw_order || it == tokens.end()) {
            if (it->type == kw_where) st_components.push_back(parse_where_clause());
            if (it->type == kw_order) st_components.push_back(parse_order_clause());
            if (it == tokens.end()) return std::make_shared<node>(statement, st_components);
        }
        else {
            std::cout << "Expected a where clause, order clause, or end of statement.\n";
            exit(1);
        }

        std::cout << "Unexpected text after statement.\n";
        exit(1);
    }

    void discard(element_type expected_type) {
        if (it->type != expected_type) {
            std::cout << "Parser error on line " << it->line_number 
                    << ". Expected a(n) " << tokenTypeToString(expected_type) 
                    << " after " << tokenTypeToString((it-1)->type) << ".\n";
            exit(1);
        }
        ++it; // consume token
    }

    void consume(element_type expected_type, std::vector<std::shared_ptr<node>>& components) {
        if (it->type != expected_type) {
            std::cout << "Parser error on line " << it->line_number 
                    << ". Expected a(n) " << tokenTypeToString(expected_type) 
                    << " after " << tokenTypeToString((it-1)->type) << ".\n";
            exit(1);
        }
        components.push_back(std::make_shared<node>(it->type));
        ++it; // consume token
    }

    void consume_optional(element_type expected_type, std::vector<std::shared_ptr<node>>& components) {
        if (it->type == expected_type) {
            components.push_back(std::make_shared<node>(it->type));
            ++it; // consume token
        }
    }

};







#endif