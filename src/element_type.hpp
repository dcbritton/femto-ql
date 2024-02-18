// element_type.hpp

#ifndef ELEMENT_TYPE
#define ELEMENT_TYPE

enum element_type {
    
    // non-terminals - specific to internal nodes
    script = 0,
    definition = 1,
    selection = 2,
    select_clause = 3,
    column_list = 4,
    from_clause = 5,
    where_clause = 6,
    bool_expr = 7,
    order_clause = 8,
    join_expr = 9,

    // terminals - tokens and leaf nodes
    kw_select = 10,           
    kw_from = 11,             
    kw_where = 12,
    kw_distinct = 13,
    kw_order = 14,
    kw_asc = 15,
    kw_desc = 16,
    kw_in = 17,
    kw_any = 18,
    kw_all = 19,
    kw_define = 20,
    kw_as = 21,
    kw_join = 22,
    kw_on = 23,
    
    identifier = 30,         // column name, table name, alias       
    int_literal = 31,
    chars_literal = 32,     // may contain anything but " because it is the delimiter
    float_literal = 33,
    kw_true = 34,           // kw_true and kw_false are placed here because they are "boolean literals"
    kw_false = 35,
    kw_null = 36,

    // punctuation 
    semicolon = 45,          
    open_parenthesis = 46,
    close_parenthesis = 47,
    asterisk = 48,
    comma = 49,

    op_equals = 50,    // do not change order of enums between equals and greater than equals. parse_bool_expr() depends on it
    op_not_equals = 51,
    op_less_than = 52,
    op_less_than_equals = 53,
    op_greater_than = 54,
    op_greater_than_equals = 55,

    op_and = 56,
    op_or = 57,
    op_not = 58
};

std::string tokenTypeToString(element_type type) {
    switch (type) {

        case script: return "script";
        case definition: return "definition";
        case selection: return "selection";
        case select_clause: return "select clause";
        case from_clause: return "from clause";
        case where_clause: return "where clause";
        case bool_expr: return "boolean expression";
        case column_list: return "column list";
        case order_clause: return "order clause";
        case join_expr: return "join expression";

        case kw_select: return "select";
        case kw_from: return "from";
        case kw_where: return "where";
        case kw_distinct: return "distinct";
        case kw_order: return "order";
        case kw_asc: return "asc";
        case kw_desc: return "desc";
        case kw_in: return "in";
        case kw_any: return "any";
        case kw_all: return "all";
        case kw_define: return "define";
        case kw_as: return "as";

        case identifier: return "identifier";
        case int_literal: return "int literal";
        case chars_literal: return "chars literal";
        case float_literal: return "float literal";
        case kw_true: return "true";
        case kw_false: return "false";
        case kw_null: return "null";

        case semicolon: return "semicolon";
        case open_parenthesis: return "open parenthesis";
        case close_parenthesis: return "close parenthesis";
        case asterisk: return "asterisk";
        case comma: return "comma";
        
        case op_equals: return "==";
        case op_not_equals: return "!=";
        case op_less_than: return "<";
        case op_less_than_equals: return "<=";
        case op_greater_than: return ">";
        case op_greater_than_equals: return ">=";

        case op_and: return "&&";
        case op_or: return "||";
        case op_not: return "!";

        default: return "Unknown type";
    }
}

#endif