// node.hpp

#ifndef NODE
#define NODE

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include "element_type.hpp"

// @TODO polymorphism
struct node {
    element_type type;
    std::string value; // empty on all but literals and identifiers (a subset non-terminal symbols)
    std::vector<std::shared_ptr<node>> components; // empty on terminal symbols
    node(element_type type) : type(type) {}; // for non-literal/identifier terminals
    node(element_type type, std::string value) : type(type), value(value) {}; // for literals and identifiers
    node(element_type type, std::vector<std::shared_ptr<node>> vec) : type(type), components(vec) {}; // for non-terminals
};

void post_order_traversal(std::shared_ptr<node> root) {
    if (root == nullptr) return;

    for (std::shared_ptr<node> child : root->components) {
       post_order_traversal(child); 
    }

    std::cout << root->type << " "; 
}

void pre_order_traversal(std::shared_ptr<node> root) {
    if (root == nullptr) return;

    std::cout << root->type << " "; 

    for (std::shared_ptr<node> child : root->components) {
        pre_order_traversal(child); 
    }
}

void print_traversals(std::shared_ptr<node> root) {
    std::cout << "Syntax tree traversals:\n------------------------\n";
    std::cout << "Pre-order: ";
    pre_order_traversal(root);
    std::cout << '\n';
    std::cout << "Post-order: ";
    post_order_traversal(root);
    std::cout << '\n';
}

#endif