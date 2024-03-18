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
                case drop:
                    validateDrop(nodePtr);
                    break;

                case creation:
                    validateCreation(nodePtr);
                    break;
                
                default:
                    std::cout << "Validation error. Tried to validate an unknown statement type: " << tokenTypeToString(nodePtr->type) << ".\n";
                    break;
            }
        }
    }

    // a million validation functions

    // validate create (original)
    void validateCreation(std::shared_ptr<node> creationRoot) {
        std::string tableName = creationRoot->components[0]->value;
        auto p = find(persistents, tableName);

        // if table already exists in permanents, can't create
        if (p != persistents.end()) {
            std::cout << "Validation error. Table \"" << tableName << "\" already exists.\n";
            exit(1);
        }

        // no <= 0 length chars
        // loop through all columns of the table, if chars, check that -> type >= 1
        for (auto columnTypePair : creationRoot->components[1]->components) {
            if (columnTypePair->components[1]->type == kw_chars) {
                if (stoi(columnTypePair->components[2]->value) <= 0) {
                    std::cout << "Validation error. Column \"" << columnTypePair->components[0]->value << "\" in created table \"" << creationRoot->components[0]->value << "\" may not have a non-positive number of columns.\n";
                    exit(1);
                }
            }
        }

        // @TODO
        // Cannot create a table from a definition that does not exist.

        // add table to permanents
        // @TODO nodeToTable(table)
        // persistents.push_back(nodeToTable(???));

        std::cout << "Creation validated.\n"; 
    }

    // validate drop statement
    void validateDrop(std::shared_ptr<node> dropRoot) {
        std::string tableName = dropRoot->components[0]->value;
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

        std::cout << "Drop statement validated.\n";
        printTableList(persistents);
        printTableList(temporaries);
        return;
    }
    
};

#endif