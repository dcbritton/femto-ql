#include <iostream>
#include <fstream>
#include <memory>
#include "token.hpp"
#include "tokenize.hpp"
#include "parse.hpp"

int main() {
    
    std::ifstream input;
    std::string statement;
    input.open("../input.fql");
    getline(input, statement, ';');
    std::cout << "Query text:\n" << statement << "\n\n";

    std::vector<token> token_stream = tokenize(statement);
    print_token_stream(token_stream);

    std::shared_ptr<node> ast = parse(token_stream);

    std::cout << '\n';
    std::cout << "Pre-order traversal: ";
    pre_order_traversal(ast);
    std::cout << '\n';

    std::cout << "Post-order traversal: ";
    post_order_traversal(ast);
    std::cout << '\n';

    std::cout << "\nProgram has ended properly.\n";
    input.close();
    return 0;
}