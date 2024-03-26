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

                case join:
                    validateJoin(nodePtr);
                    break;
                
                default:
                    std::cout << "Validator error. Tried to validate an unknown statement type: " << tokenTypeToString(nodePtr->type) << ".\n";
                    break;
            }
        }
    }

    // a million validation functions

    // validate join statement
    void validateJoin(std::shared_ptr<node> joinRoot) {
        // cannot join tables that don't exist
        std::string table1Name = joinRoot->components[0]->value;
        std::string table2Name = joinRoot->components[1]->value;

        if (!exists(table1Name, tables)) {
            std::cout << "Validator error. Table \"" << table1Name << "\" doesn't exist.\n";
            exit(1);
        }
        if (!exists(table2Name, tables)) {
            std::cout << "Validator error. Table \"" << table2Name << "\" doesn't exist.\n";
            exit(1);
        }

        // ON_EXPR
        std::shared_ptr<node> onExprRoot = joinRoot->components[2];
        std::string col1Name = onExprRoot->components[0]->value;
        auto split1 = split(col1Name);
        split1.first;
        std::string col2Name = onExprRoot->components[2]->value;
        auto split2 = split(col2Name);

        // @TODO (but handled implicitly in tokenizer, with MAX_IDENTIFIER_LENGTH)
        // new aliases cannot exceed 64 characters

        // joined columns must be in table.column form
        if (!hasDot(col1Name)) {
            std::cout << "Validator error. Column \"" << col1Name << "\" isn't in table.column form.\n";
            exit(1);
        }
        if (!hasDot(col2Name)) {
            std::cout << "Validator error. Column \"" << col1Name << "\" isn't in table.column form.\n";
            exit(1);
        }

        // verify that split1/2->first must match joined tables
        if (split1.first == joinRoot->components[0]->value && split2.first == joinRoot->components[1]->value) {}
        else if (split2.first == joinRoot->components[0]->value && split1.first == joinRoot->components[1]->value) {}
        else {
            std::cout << "Validator error. Columns to join on must reference the tables stated after \"join\"!\n";
            exit(1); 
        }

        // cannot join tables on columns that the tables don't have
        auto table1 = find(table1Name, tables);
        auto table2 = find(table2Name, tables);

        // verify that the column exists in the table
        auto it1 = std::find_if(table1->columns.begin(), table1->columns.end(),
                                [&split1](const auto& c){return c.name == split1.second;});
        if (it1 == table1->columns.end()) {
            std::cout << "Validator error. Column \"" << split1.second << "\" isn't a column in table \"" << table1->name << "\".\n";
            exit(1);
        }
        auto it2 = std::find_if(table2->columns.begin(), table2->columns.end(),
                                [&split2](const auto& c){return c.name == split2.second;});
        if (it2 == table2->columns.end()) {
            std::cout << "Validator error. Column \"" << split2.second << "\" isn't a column in table \"" << table2->name << "\".\n";
            exit(1);
        }

        // @TODO
        // joined columns must be of the same type

        // TODOS
        // require aliasing on column name conflicts
        // aliased columns must exist
        // alias names must not conflict with eachother or non-referenced columns

        std::cout << "Join validated.\n";
        printTableList(tables);
        std::cout << '\n';
    }

    // validate set operation
    void validateSetOp(std::shared_ptr<node> setOpRoot) {

        // only union/intersect tables that exist
        std::string table1Name = setOpRoot->components[1]->value;
        std::string table2Name = setOpRoot->components[2]->value;
        if (!exists(table1Name, tables)) {
            std::cout << "Validator error. Table \"" << table1Name << "\" doesn't exist.\n";
            exit(1);
        }
        if (!exists(table2Name, tables)) {
            std::cout << "Validator error. Table \"" << table2Name << "\" doesn't exist.\n";
            exit(1);
        }

        // cannot union|intersect tables with different numbers of columns
        auto first = find(table1Name, tables);
        auto second = find(table2Name, tables);
        if (first->columns.size() != second->columns.size()) {
            std::cout << "Validator error. Tables \"" << table1Name << "\" and \"" << table2Name << "\" don't have the same number of columns.\n";
            exit(1);
        }
        
        // cannot union|intersect tables with different column names
        for (int i = 0; i < first->columns.size(); ++i) {
            if (first->columns[i].name != second->columns[i].name) {
                std::cout << "Validator error. Tables \"" << table1Name << "\" and \"" << table2Name << "\" have different column names (or are in different orders).\n";
                exit(1);
            }
        }

        // cannot union|intersect tables with different column types
        for (int i = 0; i < first->columns.size(); ++i) {
            if (first->columns[i].type != second->columns[i].type) {
                std::cout << "Validator error. Tables \"" << table1Name << "\" and \"" << table2Name << "\" have different column types.\n";
                exit(1);
            }
        }

        // @TODO default to larger chars on resultant table

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
        for (auto columnTypePair : definitionRoot->components[1]->components) {
            if (columnTypePair->components[1]->type == kw_chars) {
                // check that -> type >= 1
                if (stoi(columnTypePair->components[2]->value) <= 0) {
                    std::cout << "Validation error. Column \"" << columnTypePair->components[0]->value << "\" in defined table \"" << definitionRoot->components[0]->value << "\" may not have a non-positive number of columns.\n";
                    exit(1);
                }
            }
        }

        // @TODO
        // Cannot create a table with two columns of the same name
        // Cannot create a table from a definition that does not exist.

        // add table
        tables.push_back(nodeToTable(definitionRoot));

        std::cout << "Definition validated.\n";
        printTableList(tables);
        std::cout << '\n';
    }

    // validate drop statement
    void validateDrop(std::shared_ptr<node> dropRoot) {
        std::string tableName = dropRoot->components[0]->value;

        // if table exists, drop it
        if (exists(tableName, tables)) 
            tables.erase(find(tableName, tables));

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