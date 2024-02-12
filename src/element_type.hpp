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

    identifier = 20,         // column name, table name, alias       
    int_literal = 21,        
    semicolon = 22,          
    open_parenthesis = 23,
    close_parenthesis = 24,
    asterisk = 25,
    comma = 26,

    op_equals = 30,    // do not change order of enums between equals and greater than equals. parse_bool_expr() depends on it
    op_not_equals = 31,
    op_less_than = 32,
    op_less_than_equals = 33,
    op_greater_than = 34,
    op_greater_than_equals = 35,
    op_and = 36,
    op_or = 37,
};

#endif