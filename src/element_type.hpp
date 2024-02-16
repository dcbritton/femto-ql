// element_type.hpp

#ifndef ELEMENT_TYPE
#define ELEMENT_TYPE

enum element_type {
    
    // non-terminals - specific to internal nodes
    statement = 0,
    select_clause = 1,
    from_clause = 2,
    where_clause = 3,
    bool_expr = 4,
    column_list = 5,
    order_clause = 6,

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
        case kw_select: return "kw_select";
        case kw_from: return "kw_from";
        case kw_where: return "kw_where";
        case kw_distinct: return "kw_distinct";
        case kw_order: return "kw_order";
        case kw_asc: return "kw_asc";
        case kw_desc: return "kw_desc";
        case kw_in: return "kw_in";
        case kw_any: return "kw_any";
        case kw_all: return "kw_all";

        case identifier: return "identifier";
        case int_literal: return "int_literal";
        case chars_literal: return "chars_literal";
        case float_literal: return "float_literal";
        case kw_true: return "kw_true";
        case kw_false: return "kw_false";
        case kw_null: return "kw_null";

        case semicolon: return "semicolon";
        case open_parenthesis: return "open_parenthesis";
        case close_parenthesis: return "close_parenthesis";
        case asterisk: return "asterisk";
        case comma: return "comma";
        
        case op_equals: return "op_equals";
        case op_not_equals: return "op_not_equals";
        case op_less_than: return "op_less_than";
        case op_less_than_equals: return "op_less_than_equals";
        case op_greater_than: return "op_greater_than";
        case op_greater_than_equals: return "op_greater_than_equals";

        case op_and: return "op_and";
        case op_or: return "op_or";
        case op_not: return "op_not";

        default: return "Unknown type";
    }
}

#endif