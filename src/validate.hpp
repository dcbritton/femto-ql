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
    std::vector<table> tables;

public:

    Validator(std::vector<table> initials) : tables(initials) {}
    
    // validate the AST
    void validate(std::shared_ptr<node> astRoot) {
        std::cout << "\nValidation log:\n---------------\n";
        for (auto nodePtr : astRoot->components) {
            switch (nodePtr->type) {
                // @TODO Add more validation functions
                case drop:
                    validateDrop(nodePtr);
                    break;

                case definition:
                    validateDefinition(nodePtr);
                    break;

                case set_op:
                    validateSetOp(nodePtr);
                    break;
                
                default:
                    std::cout << "Validator error. Tried to validate an unknown statement type: " << tokenTypeToString(nodePtr->type) << ".\n";
                    break;
            }
        }
    }

    // a million validation functions

    void validateSetOp(std::shared_ptr<node> setOpRoot) {
        // Only union|intersect tables that exist.
        std::string firstTableName = setOpRoot->components[1]->value;
        std::string secondTableName = setOpRoot->components[2]->value;
        if (!exists(firstTableName, tables)) {
            std::cout << "Validator error. Table \"" << firstTableName << "\" doesn't exist.\n";
            exit(1);
        }

        // Cannot union|intersect tables with different numbers of columns.
        // Cannot union|intersect tables with different column names.
        // Cannot union|intersect tables with different column types.

        std::cout << "Set operation validated.\n";
        printTableList(tables);
        std::cout << '\n';
    }

    // validate create (original)
    void validateDefinition(std::shared_ptr<node> definitionRoot) {
        // @TODO validation for definitions other than those of column, type list
        if (definitionRoot->components[1]->type != col_type_list) {
            std::cout << "Validation of a definition other than from a column, type list";
            return;
        }

        std::string tableName = definitionRoot->components[0]->value;

        // if table already exists
        if (exists(tableName, tables)) {
            std::cout << "Validation error. Table \"" << tableName << "\" already exists. Cannot define a table with the same name.\n";
            exit(1);
        }

        // no <= 0 length chars
        // loop through all columns of the table, if chars, check that -> type >= 1
        for (auto columnTypePair : definitionRoot->components[1]->components) {
            if (columnTypePair->components[1]->type == kw_chars) {
                if (stoi(columnTypePair->components[2]->value) <= 0) {
                    std::cout << "Validation error. Column \"" << columnTypePair->components[0]->value << "\" in defined table \"" << definitionRoot->components[0]->value << "\" may not have a non-positive number of columns.\n";
                    exit(1);
                }
            }
        }

        // add table
        tables.push_back(nodeToTable(definitionRoot));

        std::cout << "Definition validated.\n";
        printTableList(tables);
        std::cout << '\n';
    }

    // validate drop statement
    void validateDrop(std::shared_ptr<node> dropRoot) {
        std::string tableName = dropRoot->components[0]->value;
        auto t = find(tables, tableName);

        // if table exists, drop it
        if (t != tables.end()) 
            tables.erase(t);

        // if table is in neither, error
        else {
            std::cout << "Validation error. Table \"" << tableName << "\" doesn't exist.\n";
            exit(1);
        }

        std::cout << "Drop statement validated.\n";
        printTableList(tables);
        std::cout << '\n';
        return;
    }
    
};

#endif