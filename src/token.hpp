// token.hpp

#ifndef TOKEN
#define TOKEN

#include <string>
#include <vector>
#include "element_type.hpp"

struct token {
    element_type type;
    std::string value = "";
    unsigned int line_number = 0;
    token(element_type type, std::string value, unsigned int line_number): type(type), value(value), line_number(line_number) {};
};

#endif