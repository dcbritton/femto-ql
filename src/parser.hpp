// parser.hpp

#ifndef PARSER
#define PARSER

#include <vector>
#include <memory>
#include "token.hpp"
#include "node.hpp"

class Parser {
private:
    std::vector<token> tokens = {};
    std::vector<token>::const_iterator it;
    element_type current_non_terminal = script;
public:

    Parser(std::vector<token> token_stream) 
        : tokens(token_stream), it(tokens.begin()) {};

    // script -> [definition|selection|join_expr|set_expr]*
    std::shared_ptr<node> parse_script() {
        current_non_terminal = script;

        std::vector<std::shared_ptr<node>> script_components;
        while (it != tokens.end()) {
            if (it->type == kw_define)
                script_components.push_back(parse_definition());
            else if (it->type == kw_select)
                script_components.push_back(parse_selection());
            else if (it->type == kw_join)
                script_components.push_back(parse_join_expr());
            else if (it->type == kw_union || it->type == kw_intersect)
                script_components.push_back(parse_set_expr());
            else if (it->type == kw_insert)
                script_components.push_back(parse_insertion());
            else if (it->type == kw_update)
                script_components.push_back(parse_update_expr());
            else if (it->type == kw_delete)
                script_components.push_back(parse_deletion());
            else if (it->type == kw_create)
                script_components.push_back(parse_creation());
            else if (it->type == kw_drop)
                script_components.push_back(parse_drop_expr());
            else {
                std::cout << "Parser error on line " << it->line_number 
                          << ". Unexpected " << tokenTypeToString(it->type) << " at start/end of statement.\n";
                exit(1);
            }
        }
        return std::make_shared<node>(script, script_components);
    }

    // definition -> kw_define identifier kw_as selection|join_expr|set_expr
    std::shared_ptr<node> parse_definition() {
        current_non_terminal = definition;

        std::vector<std::shared_ptr<node>> dfn_components;
        discard(kw_define);
        consume(identifier, dfn_components);
        discard(kw_as);

        // selection
        if (it->type == kw_select)
            dfn_components.push_back(parse_selection());
        // join_expr
        else if (it->type == kw_join)
            dfn_components.push_back(parse_join_expr());
        // set_expr
        else if (it->type == kw_union || it->type == kw_intersect)
            dfn_components.push_back(parse_set_expr());
        else {
            std::cout << "Parser error on line " << it->line_number 
                      << ". Expected a selection, join expression, or set expression after as in definition.\n";
            exit(1);
        }

        return std::make_shared<node>(definition, dfn_components);
    }

    // selection -> select_clause from_clause where_clause|ε order_clause|ε
    std::shared_ptr<node> parse_selection() {
        current_non_terminal = selection;

        // required clauses
        std::vector<std::shared_ptr<node>> sln_components;
        sln_components.push_back(parse_select_clause());
        sln_components.push_back(parse_from_clause());

        // optional clauses
        if (it->type == kw_where) sln_components.push_back(parse_where_clause());
        if (it->type == kw_order) sln_components.push_back(parse_order_clause());

        return std::make_shared<node>(selection, sln_components);
    }

    // select_clause -> kw_select kw_distinct|ε column_list
    std::shared_ptr<node> parse_select_clause() {
        current_non_terminal = select_clause;

        std::vector<std::shared_ptr<node>> sc_components;
        discard(kw_select);
        consume_optional(kw_distinct, sc_components);
        sc_components.push_back(parse_column_list());

        return std::make_shared<node>(select_clause, sc_components);
    }

    // column_list -> identifier, ... identifier | *
    std::shared_ptr<node> parse_column_list() {
        current_non_terminal = column_list;

        std::vector<std::shared_ptr<node>> cl_components;
        // identifier list path
        if (it->type == identifier) {
            while (it->type == identifier && (it + 1)->type == comma) {
                consume(identifier, cl_components);
                discard(comma);
            }
            consume(identifier, cl_components);
        }
        // *
        else if (it->type == asterisk)
            consume(asterisk, cl_components);
        else {
            std::cout << "Parser error on line " << it->line_number 
                      << ". Expected a column list or * after "
                      << tokenTypeToString((it-1)->type)
                      << " in select clause.\n";
            exit(1);
        }

        return std::make_shared<node>(column_list, cl_components);
    }

    // from_clause -> kw_from identifier
    std::shared_ptr<node> parse_from_clause() {
        current_non_terminal = from_clause;

        std::vector<std::shared_ptr<node>> fc_components;
        discard(kw_from);
        consume(identifier, fc_components);

        return std::make_shared<node>(from_clause, fc_components);
    }

    // where_clause -> kw_where bool_expr
    std::shared_ptr<node> parse_where_clause() {
        current_non_terminal = where_clause;
        
        std::vector<std::shared_ptr<node>> wc_components;
        discard(kw_where);
        wc_components.push_back(parse_bool_expr());

        return std::make_shared<node>(where_clause, wc_components);
    }

    // bool_expr -> !|ε ( bool_expr ) 
    //              | identifier comparison literal 
    //              | identifier in identifier
    //              | identifier comparison any|all identifier
    //              | bool_expr bool_op bool_expr
    std::shared_ptr<node> parse_bool_expr() {
        current_non_terminal = bool_expr;

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
                    // @TODO: unexpected end of input here still results in issue #6
                    consume(it->type, lhs_components);
                }

                // any|all
                else if (it->type == kw_any || it->type == kw_all) {
                    lhs_components.push_back(std::make_shared<node>(it->type));
                    ++it; // consume kw_any/kw_all
                    consume(identifier, lhs_components);
                }
                else {
                    std::cout << "Parser error on line " << it->line_number 
                              << ". Expected a keyword any/all/null or a(n) int/float/chars/bool literal after "
                              << tokenTypeToString((it-1)->type)
                              << " in boolean expression.\n";
                    exit(1);
                }
            }
            else {
                std::cout << "Parser error on line " << it->line_number 
                          << ". Expected a keyword in or a comparison after "
                          << tokenTypeToString((it-1)->type)
                          << " in boolean expression.\n";
                exit(1);
            }

            potential_lhs = std::make_shared<node>(bool_expr, lhs_components);
        }
        else {
            std::cout << "Parser error on line " << it->line_number 
                      << ". Expected !, (, or an identifier after "
                      << tokenTypeToString((it-1)->type)
                      << " in boolean expression.\n";
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

    // order_clause -> kw_order identifier asc/desc
    std::shared_ptr<node> parse_order_clause() {
        current_non_terminal = order_clause;

        std::vector<std::shared_ptr<node>> oc_components;
        discard(kw_order);
        consume(identifier, oc_components);

        if (it->type != kw_asc && it->type != kw_desc)  {
            std::cout << "Parser error on line " << it->line_number 
                      << ". Expected a column list or * after "
                      << tokenTypeToString((it-1)->type)
                      << " in order clause.\n";
            exit(1);
        }
        oc_components.push_back(std::make_shared<node>(it->type));
        it++; // consume kw_asc/desc
            
        return std::make_shared<node>(order_clause, oc_components);
    }

    // join_expr -> kw_join identifier comma identifier on identifier comparison identifier
    std::shared_ptr<node> parse_join_expr() {
        current_non_terminal = join_expr;

        std::vector<std::shared_ptr<node>> je_components;
        discard(kw_join);
        consume(identifier, je_components);
        discard(comma);
        consume(identifier, je_components);
        je_components.push_back(parse_on_expr());

        return std::make_shared<node>(join_expr, je_components);
    }

    // on_expr -> kw_on identifier comparison identifier
    std::shared_ptr<node> parse_on_expr() {
        current_non_terminal = on_expr;

        std::vector<std::shared_ptr<node>> oe_components;
        discard(kw_on);
        consume(identifier, oe_components);

        if (it->type >= op_equals && it->type <= op_greater_than_equals) {
            oe_components.push_back(std::make_shared<node>(it->type));
            ++it; // consume comparison
        }
        else {
            std::cout << "Parser error on line " << it->line_number 
                      << ". Expected a comparison after "
                      << tokenTypeToString((it-1)->type)
                      << " in on expression.\n";
            exit(1);
        }

        consume(identifier, oe_components);

        return std::make_shared<node>(on_expr, oe_components);
    }

    // set_expr -> kw_union|kw_intersect identifier comma identifier
    std::shared_ptr<node> parse_set_expr() {
        current_non_terminal = set_expr;

        std::vector<std::shared_ptr<node>> se_components;

        if (it->type == kw_union || it->type == kw_intersect) {
            se_components.push_back(std::make_shared<node>(it->type));
            ++it; // consume union/intersect
        }
        else {
            std::cout << "Parser error on line " << it->line_number 
                      << ". Expected a union or intersect after "
                      << tokenTypeToString((it-1)->type)
                      << " in set expression.\n";
            exit(1);
        }
        consume(identifier, se_components);
        discard(comma);
        consume(identifier, se_components);

        return std::make_shared<node>(set_expr, se_components);
    }

    // deletion -> kw_delete from_clause where_clause|ε
    std::shared_ptr<node> parse_deletion() {
        current_non_terminal = deletion;

        std::vector<std::shared_ptr<node>> dc_components;
        discard(kw_delete);
        dc_components.push_back(parse_from_clause());
        if (it->type == kw_where) dc_components.push_back(parse_where_clause());

        return std::make_shared<node>(deletion, dc_components);
    }

    // update_expr -> kw_update identifier colon col_val_list where_clause
    std::shared_ptr<node> parse_update_expr() {
        current_non_terminal = update_expr;

        std::vector<std::shared_ptr<node>> ue_components;
        discard(kw_update);
        consume(identifier, ue_components);
        discard(colon);
        ue_components.push_back(parse_col_val_list());
        ue_components.push_back(parse_where_clause());

        return std::make_shared<node>(update_expr, ue_components);
    }
    
    // insertion -> kw_insert kw_into identifier colon col_val_list
    std::shared_ptr<node> parse_insertion() {
        current_non_terminal = insertion;

        std::vector<std::shared_ptr<node>> insert_components;
        discard(kw_insert);
        discard(kw_into);
        consume(identifier, insert_components);
        discard(colon);
        insert_components.push_back(parse_col_val_list());

        return std::make_shared<node>(insertion, insert_components);
    }

    // col_val_list -> col_val, ... col_val
    std::shared_ptr<node> parse_col_val_list() {
        current_non_terminal = col_val_list;

        std::vector<std::shared_ptr<node>> cvl_components;
        bool cvl = true;
        while(cvl){
            cvl_components.push_back(parse_col_val());
            if (it->type == comma)
                discard(comma);
            else 
                cvl = false;
        }
        
        return std::make_shared<node>(col_val_list, cvl_components);
    }

    // col_val -> identifier(literal)
    std::shared_ptr<node> parse_col_val() {
        current_non_terminal = col_val;

        std::vector<std::shared_ptr<node>> cv_components;
        consume(identifier, cv_components);
        discard(open_parenthesis);
        // literal (incl. null)
        if (it->type >= int_literal && it->type <= kw_null)
            // @TODO: unexpected end of input here still results in issue #6
            consume(it->type, cv_components);
        else {
            std::cout << "Parser error on line " << it->line_number 
                      << ". Expected a literal after "
                      << tokenTypeToString((it-1)->type)
                      << " in column, value pair.\n";
            exit(1);
        }
        discard(close_parenthesis);

        return std::make_shared<node>(col_val, cv_components);
    }

    // creation -> kw_create identifier :col_type_list|ε
    std::shared_ptr<node> parse_creation() {
        current_non_terminal = creation;

        std::vector<std::shared_ptr<node>> crn_components;
        discard(kw_create);
        consume(identifier, crn_components);
        if (it->type == colon) {
            discard(colon);
            crn_components.push_back(parse_col_type_list());
        }

        return std::make_shared<node>(creation, crn_components);
    }

    // col_type_list -> col_type, ... col_type
    std::shared_ptr<node> parse_col_type_list() {
        current_non_terminal = col_type_list;

        std::vector<std::shared_ptr<node>> ctl_components;
        bool ctl = true;
        while(ctl) {
            ctl_components.push_back(parse_col_type());
            if (it->type == comma)
                discard(comma);
            else 
                ctl = false;
        }

        return std::make_shared<node>(col_type_list, ctl_components);
    }

    // col_type -> identifier(int|float|bool| chars int_literal)
    std::shared_ptr<node> parse_col_type() {
        current_non_terminal = col_type;

        std::vector<std::shared_ptr<node>> ct_components;
        consume(identifier, ct_components);
        discard(open_parenthesis);
        // type
        if (it->type >= kw_int && it->type <= kw_chars) {
            // @TODO: unexpected end of input here still results in issue #6
            consume(it->type, ct_components);
            // if it was chars, then consume the number
            if ((it-1)->type == kw_chars)
                consume(int_literal, ct_components);
        }
        else {
            std::cout << "Parser error on line " << it->line_number 
                      << ". Expected a type after "
                      << tokenTypeToString((it-1)->type)
                      << " in column, type pair.\n";
            exit(1);
        }
        discard(close_parenthesis);

        return std::make_shared<node>(col_type, ct_components);
    }

    // drop_expr -> kw_drop identifier
    std::shared_ptr<node> parse_drop_expr() {
        current_non_terminal = drop_expr;

        std::vector<std::shared_ptr<node>> de_components;
        discard(kw_drop);
        consume(identifier, de_components);

        return std::make_shared<node>(drop_expr, de_components);
    }

    void discard(element_type expected_type) {

        if (it == tokens.end()) {
            std::cout << "Parser error on line " << (it-1)->line_number
                      << ". Unexpected end of input after " << tokenTypeToString((it-1)->type) 
                      << " in " <<  tokenTypeToString(current_non_terminal) << ".\n";
            exit(1);
        }

        if (it->type != expected_type) {
            std::cout << "Parser error on line " << it->line_number 
                      << ". Expected a(n) " << tokenTypeToString(expected_type) 
                      << " after " << tokenTypeToString((it-1)->type)
                      << " in " << tokenTypeToString(current_non_terminal) << ".\n";
            exit(1);
        }
        ++it; // consume token
    }

    void consume(element_type expected_type, std::vector<std::shared_ptr<node>>& components) {

        if (it == tokens.end()) {
            std::cout << "Parser error on line " << (it-1)->line_number
                      << ". Unexpected end of input after " << tokenTypeToString((it-1)->type) 
                      << " in " << tokenTypeToString(current_non_terminal) << ".\n";
            exit(1);
        }

        if (it->type != expected_type) {
            std::cout << "Parser error on line " << it->line_number 
                      << ". Expected a(n) " << tokenTypeToString(expected_type) 
                      << " after " << tokenTypeToString((it-1)->type)
                      << " in " << tokenTypeToString(current_non_terminal) << ".\n";
            exit(1);
        }

        // use constructor with value for identifiers and literals
        if (it->type >= identifier && it->type <= float_literal) {
            components.push_back(std::make_shared<node>(it->type, it->value));
            ++it; // consume token
            return;
        }

        components.push_back(std::make_shared<node>(it->type));
        ++it; // consume token
    }

    void consume_optional(element_type expected_type, std::vector<std::shared_ptr<node>>& components) {

        if (it == tokens.end()) {
            std::cout << "Parser error on line " << (it-1)->line_number
                      << ". Unexpected end of input after " << tokenTypeToString((it-1)->type) 
                      << " in " << current_non_terminal << ".\n";
            exit(1);
        }

        if (it->type == expected_type) {
            components.push_back(std::make_shared<node>(it->type));
            ++it; // consume token
        }
    }

};

#endif