// token.hpp

#ifndef TOKEN
#define TOKEN

#include <string>
#include <vector>
#include "element_type.hpp"

struct token {
    element_type type;
    std::string value = "";
    token(element_type type, std::string value) : type(type), value(value) {}; // member initialization list :)
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