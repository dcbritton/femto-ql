// node.hpp

#ifndef NODE
#define NODE

#include <iostream>
#include <string>
#include <vector>

enum node_type {
    statement,
    select_clause,
    from_clause,
    where_clause,
    bool_expr,           
    node_identifier, // column name, table name, alias       
    node_op_equals,
    node_int_literal
};

// this feels like a use for polymorphism -_-
struct node {
    node_type type;
    std::string value; // empty on non-terminal symbols
    std::vector<node*> components; // empty on terminal symbols
    node(node_type type) : type(type) {};
};

void post_order_traversal(node* root) {
    if (root == nullptr) return;

    for (node* child : root->components) {
       post_order_traversal(child); 
    }

    std::cout << root->type << " "; 
}

void pre_order_traversal(node* root) {
    if (root == nullptr) return;

    std::cout << root->type << " "; 

    for (node* child : root->components) {
        pre_order_traversal(child); 
    }
}

#endif