#include <iostream>
#include "token.hpp"
#include "tokenize.hpp"

int main() {
    std::cout << "Enter your query: ";
    std::string statement;
    getline(std::cin, statement);

    std::vector<token> token_stream = tokenize(statement);
    print_token_stream(token_stream);

    std::cout << "Program has ended properly.\n";
    return 0;
}