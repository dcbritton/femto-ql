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
        std::string firstTableName = joinRoot->components[0]->value;
        std::string secondTableName = joinRoot->components[1]->value;

        if (!exists(firstTableName, tables)) {
            std::cout << "Validator error. Table \"" << firstTableName << "\" doesn't exist.\n";
            exit(1);
        }
        if (!exists(secondTableName, tables)) {
            std::cout << "Validator error. Table \"" << secondTableName << "\" doesn't exist.\n";
            exit(1);
        }

        // ON_EXPR
        std::shared_ptr<node> onExprRoot = joinRoot->components[2];

        // @TODO (but handled implicitly in tokenizer, with MAX_IDENTIFIER_LENGTH)
        // new aliases cannot exceed 64 characters

        // cannot join tables on columns that don't exist

        // joined column alias must not conflict with other column names
        std::string joinedColumnAlias = onExprRoot->components[3]->value;
        auto first = find(firstTableName, tables);
        auto second = find(secondTableName, tables);
        for (const auto& col : first->columns)
            if (col.name == joinedColumnAlias) {
                std::cout << "Validator error. Joined column alias \"" << joinedColumnAlias << "\" already exists in " << first->name << ".\n";
                exit(1);
            }
        for (const auto& col : second->columns)
            if (col.name == joinedColumnAlias) {
                std::cout << "Validator error. Joined column alias \"" << joinedColumnAlias << "\" already exists in " << second->name << ".\n";
                exit(1);
            }

        // these columns must me in table.column form.
        // joined columns must be of the same type.

        // require aliasing on column name conflicts
        // aliased columns must exist

        std::cout << "Join validated.\n";
        printTableList(tables);
        std::cout << '\n';
    }

    // validate set operation
    void validateSetOp(std::shared_ptr<node> setOpRoot) {

        // only union/intersect tables that exist
        std::string firstTableName = setOpRoot->components[1]->value;
        std::string secondTableName = setOpRoot->components[2]->value;
        if (!exists(firstTableName, tables)) {
            std::cout << "Validator error. Table \"" << firstTableName << "\" doesn't exist.\n";
            exit(1);
        }
        if (!exists(secondTableName, tables)) {
            std::cout << "Validator error. Table \"" << secondTableName << "\" doesn't exist.\n";
            exit(1);
        }

        // cannot union|intersect tables with different numbers of columns
        auto first = find(firstTableName, tables);
        auto second = find(secondTableName, tables);
        if (first->columns.size() != second->columns.size()) {
            std::cout << "Validator error. Tables \"" << firstTableName << "\" and \"" << secondTableName << "\" don't have the same number of columns.\n";
            exit(1);
        }
        
        // cannot union|intersect tables with different column names
        for (int i = 0; i < first->columns.size(); ++i) {
            if (first->columns[i].name != second->columns[i].name) {
                std::cout << "Validator error. Tables \"" << firstTableName << "\" and \"" << secondTableName << "\" have different column names (or are in different orders).\n";
                exit(1);
            }
        }

        // cannot union|intersect tables with different column types
        for (int i = 0; i < first->columns.size(); ++i) {
            if (first->columns[i].type != second->columns[i].type) {
                std::cout << "Validator error. Tables \"" << firstTableName << "\" and \"" << secondTableName << "\" have different column types.\n";
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