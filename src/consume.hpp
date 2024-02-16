// consume.hpp

#ifndef CONSUME
#define CONSUME

#include "token.hpp"
#include "element_type.hpp"
#include <vector>
#include <memory>


void consume(element_type expected_type, std::vector<std::shared_ptr<node>>& components, std::vector<token>::const_iterator& it) {
    if (it->type != expected_type) {
        std::cout << "Expected token: " << tokenTypeToString(expected_type) << ".\n";
        exit(1);
    }
    components.push_back(std::make_shared<node>(it->type));
    ++it; // consume token
}

void consume_optional(element_type expected_type, std::vector<std::shared_ptr<node>>& components, std::vector<token>::const_iterator& it) {
    if (it->type == expected_type) {
        components.push_back(std::make_shared<node>(it->type));
        ++it; // consume token
    }
}

#endif  