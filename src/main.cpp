#include <iostream>
#include <memory>
#include "token.hpp"
#include "tokenize.hpp"
#include "parse.hpp"

int main() {
    std::cout << "Enter your query: ";
    std::string statement;
    getline(std::cin, statement);

    std::vector<token> token_stream = tokenize(statement);
    print_token_stream(token_stream);

    std::shared_ptr<node> ast = parse(token_stream);

    pre_order_traversal(ast);
    std::cout << '\n';
    post_order_traversal(ast);
    std::cout << '\n';

    std::cout << "Program has ended properly.\n";

    return 0;
}