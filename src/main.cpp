#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>
#include "token.hpp"
#include "tokenize.hpp"
#include "parser.hpp"

int main() {
    
    std::ifstream input;
    std::stringstream s;
    std::string script;
    input.open("../input.fql");
    s << input.rdbuf();
    script = s.str();

    remove_comments(script);
    print_escaped_whitespace(script);

    std::vector<token> token_stream = tokenize(script);
    print_token_stream(token_stream);

    Parser p(token_stream);
    std::shared_ptr<node> ast = p.parse_script();

    std::cout << "Syntax tree traversals:\n------------------------\n";
    std::cout << "Pre-order: ";
    pre_order_traversal(ast);
    std::cout << '\n';

    std::cout << "Post-order: ";
    post_order_traversal(ast);
    std::cout << '\n';

    std::cout << "\nProgram has ended properly.\n";
    input.close();
    return 0;
}