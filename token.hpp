// token.hpp

#ifndef TOKEN
#define TOKEN

#include <string>
#include <vector>

enum token_type {          
    kw_select,           
    kw_from,             
    kw_where,           
    identifier,         // column name, table name, alias       
    operator_equals,
    int_literal,        
    semicolon,          
    open_parenthesis,
    close_parenthesis
};

struct token {
    token_type type;
    std::string value = "";
    token(token_type type, std::string value) : type(type), value(value) {}; // member initialization list :)
};

void print_token_stream(std::vector<token> token_stream) {
    for (auto x : token_stream) 
        std::cout << x.value << ' ';
    std::cout << '\n';
    for (auto x : token_stream) 
        std::cout << x.type << ' ';
    std::cout << '\n';
}

#endif