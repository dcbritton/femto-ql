#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>
#include "token.hpp"
#include "tokenize.hpp"
#include "parser.hpp"
#include "graph_viz.hpp"
#include "validate.hpp"

int main() {
    
    // get input
    std::ifstream input;
    std::stringstream s;
    std::string script;
    input.open("../input.fql");
    s << input.rdbuf();
    script = s.str();

    // remove comments and print resultant text
    remove_comments(script);
    std::cout << '\n';
    print_escaped_whitespace(script);

    // tokenize and print tokens
    std::vector<token> token_stream = tokenize(script);
    print_token_stream(token_stream);

    // parse
    Parser p(token_stream);
    std::shared_ptr<node> ast = p.parse_script();

    // traverse syntax trees
    std::cout << "Syntax tree traversals:\n------------------------\n";
    std::cout << "Pre-order: ";
    pre_order_traversal(ast);
    std::cout << '\n';
    std::cout << "Post-order: ";
    post_order_traversal(ast);
    std::cout << '\n';

    // make graphviz  output
    make_dotfile(ast, "../dotfile.txt");

    // get and print current state of tables
    std::cout << '\n';
    std::vector<table> symbolTable = buildTableList("../tables");
    printTableList(symbolTable);

    std::cout << "\nProgram has ended properly.\n";
    input.close();
    return 0;
}