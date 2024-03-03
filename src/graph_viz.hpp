// graphviz.hpp

#ifndef GRAPHVIZ
#define GRAPHVIZ

#include <fstream>
#include <iostream>
#include "node.hpp"

int global_id = 0; // each node needs a unique id for graphviz

//
void traverse(const std::shared_ptr<node>& current_node, std::ofstream& out) {
    // base case
    if (!current_node) return;

    // create node
    int node_id = global_id++;
    out << "    node" << node_id << " [label=\"";
    // on non-literal/non-identifiers use type
    if (current_node->value == "")
        out <<  tokenTypeToString(current_node->type);
    // on literals and identifiers use value
    else {
        // for chars literal, add "
        if (current_node->type == chars_literal) {
            out << "\\\"" << current_node->value << "\\\"";
        }
        else
            out << current_node->value;
    }
    out << "\"];\n";

    //  for each child, traverse() and connect back to this node
    for (auto& child : current_node->components) {
        int child_id = global_id;
        traverse(child, out); 
        // connect current node to its child
        out << "    node" << node_id << " -- node" << child_id << ";\n"; 
    }
}

// create dotfile.txt and begin traversal
void make_dotfile(const std::shared_ptr<node>& root, const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cout << "Error. Couldn't open output file: " << filename << '\n';
        return;
    }

    file << "graph G {\n";
    traverse(root, file);
    file << "}\n";
    file.close();
}

#endif