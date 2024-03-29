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
    input.close();
    script = s.str();

    // remove comments and print resultant text
    remove_comments(script);
    // print_escaped_whitespace(script);

    // tokenize and print tokens
    std::vector<token> token_stream = tokenize(script);
    print_token_stream(token_stream);

    // parse
    Parser p(token_stream);
    std::shared_ptr<node> ast = p.parse_script();

    // traverse syntax trees
    print_traversals(ast);

    // make graphviz output
    make_dotfile(ast, "../dotfile.txt");

    // get and print current state of tables
    std::vector<table> initialTables = buildTableList("../tables");
    std::cout << "\nCurrent tables:\n---------------\n";
    printTableList(initialTables);
    
    // validate
    Validator v(initialTables);
    v.validate(ast);

    std::cout << "\nProgram has ended properly.\n";
    return 0;
}