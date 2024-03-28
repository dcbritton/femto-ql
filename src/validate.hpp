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

                case insertion:
                    validateInsertion(nodePtr);
                    break;
                
                default:
                    std::cout << "Validator error. Tried to validate an unknown statement type: " << tokenTypeToString(nodePtr->type) << ".\n";
                    break;
            }
        }
    }

    // a million validation functions

    // validate insertion
    void validateInsertion(std::shared_ptr<node> insertionRoot) {
        auto columnValueListRoot = insertionRoot->components[1];

        // table must exist
        std::string tableName = insertionRoot->components[0]->value;
        if (!exists(tableName, tables)) {
            std::cout << "Validator error. Table \"" << tableName << "\" to insert values into does not exist.\n";
            exit(1);
        }

        // columns in column-value list must exist in table
        auto t = find(tableName, tables);
        for (auto& columnValuePair : columnValueListRoot->components) {
            if (!exists(columnValuePair->components[0]->value , t->columns)) {
                std::cout << "Validator error. Column \"" << columnValuePair->components[0]->value << "\" does not exist in table \"" << t->name << "\".\n";
                exit(1);     
            }
        }

        // cannot insert a value of the wrong type into the column
        std::vector<element_type> elementTypes = {int_literal, float_literal, chars_literal, bool_literal};
        for (auto& columnValuePair : columnValueListRoot->components) {
            auto c = find(columnValuePair->components[0]->value, t->columns);
            element_type pairType = columnValuePair->components[1]->type;
            std::string pairValue = columnValuePair->components[1]->value;

            // a null can be inserted into any column
            if (pairType != kw_null)
                continue;

            for (const element_type& literal_type : elementTypes) {
                // ex: if the node is an int literal, the column type must also be an int literal
                if (c->type == literal_type && pairType != literal_type) {
                    std::cout << "Validation error. Column \"" << t->name + '.' + c->name << "\" is of type " << c->type << ", but an insert of "
                            << tokenTypeToString(pairType) << " " << pairValue << " was attempted.\n";
                    exit(1);
                }
            }
        }

        // cannot insert into the same column twice

        // Cannot insert more chars than the max length for that column.

        // @NOTE unmentioned columns assume null insert.

        std::cout << "Insert validated.\n";
        printTableList(tables);
        std::cout << '\n';
    }

    // validate join statement
    void validateJoin(std::shared_ptr<node> joinRoot) {

        // cannot join tables that don't exist
        std::string table1Name = joinRoot->components[0]->value;
        if (!exists(table1Name, tables)) {
            std::cout << "Validator error. Table \"" << table1Name << "\" doesn't exist.\n";
            exit(1);
        }
        std::string table2Name = joinRoot->components[1]->value;
        if (!exists(table2Name, tables)) {
            std::cout << "Validator error. Table \"" << table2Name << "\" doesn't exist.\n";
            exit(1);
        }

        // new aliases cannot exceed 64 characters
        // handled implicitly in tokenizer, with MAX_IDENTIFIER_LENGTH

        // ON_EXPR
        std::shared_ptr<node> onExprRoot = joinRoot->components[2];

        // joined columns must be in table.column form
        std::string col1Name = onExprRoot->components[0]->value;
        if (!hasDot(col1Name)) {
            std::cout << "Validator error. Column \"" << col1Name << "\" isn't in table.column form.\n";
            exit(1);
        }
        std::string col2Name = onExprRoot->components[2]->value;
        if (!hasDot(col2Name)) {
            std::cout << "Validator error. Column \"" << col1Name << "\" isn't in table.column form.\n";
            exit(1);
        }

        // verify that joined column names reference joined tables
        auto split1 = split(col1Name);
        auto split2 = split(col2Name);
        if (split1.first == joinRoot->components[0]->value && split2.first == joinRoot->components[1]->value) {}
        else if (split2.first == joinRoot->components[0]->value && split1.first == joinRoot->components[1]->value) {}
        else {
            std::cout << "Validator error. Columns to join on must reference the tables stated after \"join\"!\n";
            exit(1); 
        }

        // @TODO use overloaded exists()
        // cannot join tables on columns that the tables don't have
        auto table1 = find(table1Name, tables);
        auto table2 = find(table2Name, tables);
        // verify that the column exists in the table
        auto col1 = std::find_if(table1->columns.begin(), table1->columns.end(), [&split1](const auto& c){return c.name == split1.second;});
        if (col1 == table1->columns.end()) {
            std::cout << "Validator error. Column \"" << split1.second << "\" isn't a column in table \"" << table1->name << "\".\n";
            exit(1);
        }
        auto col2 = std::find_if(table2->columns.begin(), table2->columns.end(), [&split2](const auto& c){return c.name == split2.second;});
        if (col2 == table2->columns.end()) {
            std::cout << "Validator error. Column \"" << split2.second << "\" isn't a column in table \"" << table2->name << "\".\n";
            exit(1);
        }

        // joined columns must be of the same type
        if (col1->type != col2->type) {
            std::cout << "Validator error. Column \"" << col1Name << "\" is type " << col1->type << ", and \"" << col2Name << "\" is type " << col2->type << ".\n";
            exit(1);
        }

        // require aliasing on joined column name conflict
        if (col1->name != col2->name && onExprRoot->components[3]->type == nullnode) {
            std::cout << "Validator error. Joined columns \"" << col1Name << "\" and \"" << col2Name << "\" do not have the same name and must be aliased.\n";
            exit(1);  
        }

        // @TODO make sure the above alias does not conflict with any other columns

        // make sure the above alias is not in table.column form
        if (onExprRoot->components[3]->type == nullnode && hasDot(onExprRoot->components[3]->value)) {
            std::cout << "Validator error. The column alias \"" << onExprRoot->components[3]->value << "\" should not be in table.column form.\n";
            exit(1);   
        }

        // ALIAS LIST
        std::shared_ptr<node> aliasListRoot = joinRoot->components[3];

        // require aliasing on column name conflicts (excluding joined columns)
        std::vector<std::string> conflictingColumnNames;
        // identify all conflicting names non-joined column
        for (auto& col : table1->columns) {
            if (exists(col.name, table2->columns) && col.name != col1->name) 
                conflictingColumnNames.push_back(col.name);
        }
        // if there is at least one name conflict, make sure alias list is not nullnode
        if (conflictingColumnNames.size() != 0 && aliasListRoot->type == nullnode) {
            std::cout << "Validator error. There are name conflicts in non-joined columns, but no alias list.\n";
            exit(1);
        }
        // for each conflicting name, check that "(table1||table2).name" is aliased
        for (auto& name : conflictingColumnNames) {
            // need to find in aliasList where alias->components[0]->value == "(table1||table2).name".
            auto it_alias = std::find_if(aliasListRoot->components.begin(), aliasListRoot->components.end(), 
                                        [&name, &table1, &table2](const auto& aliasRoot){
                                            return aliasRoot->components[0]->value == table1->name + '.' + name ||  aliasRoot->components[0]->value == table2->name + '.' + name; 
                                        });
            // if not found require alias
            if (it_alias == aliasListRoot->components.end()) {
                std::cout << "Validator error. Column \"" << name << "\" needs an alias on table \"" << table1->name << "\" or table \"" << table2->name << "\".\n";
                exit(1);
            }
        }

        // all alias->components[0] must be in table.column form
        for (auto& aliasRoot : aliasListRoot->components) {
            if(!hasDot(aliasRoot->components[0]->value)) {
                std::cout << "Validator error. Aliased column \"" << aliasRoot->components[0]->value << "\" is not in table.column form.\n";
                exit(1);
            }
        }

        // all alias->components[1] must NOT be in table.column form
        for (auto& aliasRoot : aliasListRoot->components) {
            if(hasDot(aliasRoot->components[1]->value)) {
                std::cout << "Validator error. Alias \"" << aliasRoot->components[1]->value << "\" must not be in table.column form.\n";
                exit(1);
            }
        }

        // that table must be either table1 or table2
        // cannot alias columns that the tables don't have
        for (auto& aliasRoot : aliasListRoot->components) {
            auto aliasedName = split(aliasRoot->components[0]->value);

            // find which table to use
            auto it_table = tables.cend();
            if (aliasedName.first == table1->name)
                it_table = table1;
            else if (aliasedName.first == table2->name)
                it_table = table2;
            // in neither
            else {
                std::cout << "Validator error. Aliased column \"" << aliasRoot->components[0]->value << "\" references table \"" << aliasedName.first
                          << "\", which is neither of the joined tables.\n";
                exit(1);
            }
            
            // check if the column is in the table
            if (!exists(aliasedName.second, it_table->columns)) {
                std::cout << "Validator error. Aliased column \"" << aliasRoot->components[0]->value << "\" doesn't exist.\n";
                exit(1);
            }

        }

        // not more than one alias for the same column
        std::vector<std::string> names;
        for (auto& aliasRoot : aliasListRoot->components) {
            std::string aliasedName = aliasRoot->components[0]->value;
            if (std::find(names.begin(), names.end(), aliasedName) != names.end()) {
                std::cout << "Validator error. Multiple aliases of column \"" << aliasedName << "\".\n";
                exit(1);
            }
            names.push_back(aliasedName);
        }

        // cannot do an alias of the joined columns in the alias list
        for (auto& aliasRoot : aliasListRoot->components) {
            std::string aliasedName = split(aliasRoot->components[0]->value).second;
            std::string aliasName = aliasRoot->components[1]->value;
            if (aliasedName == col1->name || aliasedName == col2->name) {
                std::cout << "Validator error. Cannot apply the alias \"" << aliasName << "\" to \"" << aliasRoot->components[0]->value << "\" because it is one of the joined columns.\n";
                exit(1);
            }
        }

        // @TODO the below approaches disallow using names that have been aliased away
        // e.g. if t1.x is aliased to x1, t2.y cannot be aliased to x
        // allow that kind of thing (low-priority, this could cause headaches anyways)

        // if it exists, make sure the on_expr alias does not conflict with ANY columns in table1 and table2 other than the joined columns
        if (onExprRoot->components[3]->type != nullnode) {
            std::string onExprAlias = onExprRoot->components[3]->value;
            if (!(onExprAlias == col1->name || onExprAlias == col2->name)) {
                if (exists(onExprAlias, table1->columns)) {
                    std::cout << "Validator error. Alias \"" << onExprAlias << "\" conflicts with a column in table \"" << table1->name << "\".\n";
                    exit(1);
                }
                if (exists(onExprAlias, table2->columns)) {
                    std::cout << "Validator error. Alias \"" << onExprAlias << "\" conflicts with a column in table \"" << table2->name << "\".\n";
                    exit(1);
                }
            }
        }

        // alias names must not conflict with eachother, columns in table1 or table2, or the on_expr alias
        std::vector<std::string> aliases;
        for (auto& aliasRoot : aliasListRoot->components) {
            std::string aliasName = aliasRoot->components[1]->value;

            // conflicting aliases
            if (std::find(aliases.begin(), aliases.end(), aliasName) != aliases.end()) {
                std::cout << "Validator error. Attempted to alias two columns to the same name: \"" << aliasName << "\".\n";
                exit(1);
            }
            aliases.push_back(aliasName);

            // conflict with a column in table1 or table2
            if (exists(aliasName, table1->columns)) {
                std::cout << "Validator error. Alias \"" << aliasName << "\" of column \"" << aliasRoot->components[0]->value 
                          << "\" conflicts with column \"" << aliasName << "\" in table \"" << table1->name << "\".\n";
                exit(1);
            }
            if (exists(aliasName, table2->columns)) {
                std::cout << "Validator error. Alias \"" << aliasName << "\" of column \"" << aliasRoot->components[0]->value 
                          << "\" conflicts with column \"" << aliasName << "\" in table \"" << table2->name << "\".\n";
                exit(1);
            }

            // if it exists, handle conflict with on_expr alias
            if (onExprRoot->components[3]->type != nullnode) {
                std::string onExprAlias = onExprRoot->components[3]->value;
                if (aliasName == onExprAlias) {
                    std::cout << "Validator error. Alias \"" << aliasName << "\" of column \"" << aliasRoot->components[0]->value
                              << "\" conflicts with the alias of joined columns \"" << onExprAlias << "\".\n";
                    exit(1);
                }
            }

        }

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
        if (definitionRoot->components[2]->type != col_type_list) {
            std::cout << "Validation of a definition other than from a column, type list";
            return;
        }

        std::string tableName = definitionRoot->components[1]->value;

        // if table already exists
        if (exists(tableName, tables)) {
            std::cout << "Validation error. Table \"" << tableName << "\" already exists. Cannot define a table with the same name.\n";
            exit(1);
        }

        // no <= 0 length chars
        for (auto columnTypePair : definitionRoot->components[2]->components) {
            if (columnTypePair->components[1]->type == kw_chars) {
                // check that -> type >= 1
                if (stoi(columnTypePair->components[2]->value) <= 0) {
                    std::cout << "Validation error. Column \"" << columnTypePair->components[0]->value << "\" in defined table \"" << definitionRoot->components[1]->value << "\" may not have a non-positive number of columns.\n";
                    exit(1);
                }
            }
        }

        // @TODO
        // Cannot create a table with two columns of the same name

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