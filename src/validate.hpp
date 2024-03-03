// validate.hpp

#ifndef VALIDATE
#define VALIDATE

#include <fstream>
#include <iostream>
#include <filesystem>
#include <algorithm>
#include "table.hpp"
#include "node.hpp"

class Validator {
private:
    std::vector<table> persistents;
    std::vector<table> temporaries;

public:

    Validator(std::vector<table> initialSymbols) {
        persistents = initialSymbols;
    }
    
    // validate the AST
    void validate(std::shared_ptr<node> astRoot) {
        std::cout << "\nValidation log:\n---------------\n";
        for (auto nodePtr : astRoot->components) {
            switch (nodePtr->type) {
                // @TODO Add more validation functions
                case drop_expr:
                    validateDropExpr(nodePtr);
                    break;
                
                default:
                    std::cout << "Validation error. Tried to validate an unknown expression type: " << tokenTypeToString(nodePtr->type) << ".\n";
                    break;
            }
        }
    }

    // a million validation functions

    // validate drop expressions
    void validateDropExpr(std::shared_ptr<node> dropExpr) {
        std::string tableName = dropExpr->components[0]->value;
        auto p = find(persistents, tableName);
        auto t = find(temporaries, tableName);

        // if table is in permanents, drop it
        if (p != persistents.end()) 
            persistents.erase(p);

        // if table is in temporaries, drop it
        else if (t != temporaries.end()) 
            temporaries.erase(t);

        // if table is in neither, error
        else {
            std::cout << "Validation error. Table \"" << tableName << "\" exists neither as a permanent nor temporary table.\n";
            exit(1);
        }

        std::cout << "Drop expression validated.\n";
        printTableList(persistents);
        printTableList(temporaries);
        return;
    }
    
};

#endif