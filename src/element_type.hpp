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
    on_expr = 10,
    set_expr = 11,
    deletion = 12,

    // terminals - tokens and leaf nodes
    // keywords
    kw_select = 20,
    kw_from = 21,
    kw_where = 22,
    kw_distinct = 23,
    kw_order = 24,
    kw_asc = 25,
    kw_desc = 26,
    kw_in = 27,
    kw_any = 28,
    kw_all = 29,

    kw_define = 30,
    kw_as = 31,
    kw_join = 32,
    kw_on = 33,
    kw_union = 34,
    kw_intersect = 35,

    kw_delete = 36,
    
    // identifiers and literals
    identifier = 50,         // column name, table name, alias       
    int_literal = 51,
    chars_literal = 52,     // may contain anything but " because it is the delimiter
    float_literal = 53,
    kw_true = 54,           // kw_true and kw_false are placed here because they are "boolean literals"
    kw_false = 55,
    kw_null = 56,

    // punctuation 
    semicolon = 70,          
    open_parenthesis = 71,
    close_parenthesis = 72,
    asterisk = 73,
    comma = 74,
    
    // operators
    op_equals = 80,    // do not change order of enums between equals and greater than equals. parse_bool_expr() depends on it
    op_not_equals = 81,
    op_less_than = 82,
    op_less_than_equals = 83,
    op_greater_than = 84,
    op_greater_than_equals = 85,

    op_and = 97,
    op_or = 98,
    op_not = 99
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
        case on_expr: return "on expression";
        case set_expr: return "set expression";

        case deletion: return "deletion";

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
        case kw_join: return "join";
        case kw_on: return "on";
        case kw_union: return "union";
        case kw_intersect: return "intersect";

        case kw_delete: return "delete";

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