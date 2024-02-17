#include <iostream>
#include <fstream>
#include <memory>
#include "token.hpp"
#include "tokenize.hpp"
#include "parser.hpp"

int main() {
    
    std::ifstream input;
    std::string statement;
    input.open("../input.fql");
    getline(input, statement, ';');
    // std::cout << "Query text:\n" << statement << "\n\n";

    remove_comments(statement);
    std::vector<token> token_stream = tokenize(statement);
    print_token_stream(token_stream);

    Parser p(token_stream);

    std::cout << '\n';
    std::shared_ptr<node> ast = p.parse_script();

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