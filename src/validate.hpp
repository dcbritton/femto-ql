// validate.hpp

#ifndef VALIDATE
#define VALIDATE

#include <fstream>
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <map>
#include "TableInfo.hpp"
#include "node.hpp"

class Validator {
private:
    std::vector<TableInfo> tables;
    TableInfo workingTable = TableInfo();

public:

    Validator(std::vector<TableInfo> initials) : tables(initials) {}
    
    // validate the AST
    void validate(std::shared_ptr<node> astRoot) {
        // std::cout << "\nValidation log:\n---------------\n";
        for (auto nodePtr : astRoot->components) {
            switch (nodePtr->type) {
                // @TODO Add more validation functions
                case drop:
                    validateDrop(nodePtr);
                    break;

                case definition:
                    validateDefinition(nodePtr);
                    break;

                case bag_op:
                    validateBagOp(nodePtr);
                    break;

                case join:
                    validateJoin(nodePtr);
                    break;

                case insertion:
                    validateInsertion(nodePtr);
                    break;

                case update:
                    validateUpdate(nodePtr);
                    break;

                case selection:
                    validateSelection(nodePtr);
                    break;
                
                case deletion:
                    validateDeletion(nodePtr);
                    break;

                default:
                    std::cout << "Validator error. Tried to validate an unknown statement type: " << tokenTypeToString(nodePtr->type) << ".\n";
                    break;
            }
        }
    }

    // validate deletion
    void validateDeletion(std::shared_ptr<node> deletionRoot) {
        
        // table must exist
        std::string tableName = deletionRoot->components[0]->value;
        if (!exists(tableName, tables)) {
            std::cout << "Validator error. Attempted deletion within table \"" << tableName << "\", which does not exist.\n";
            exit(1);
        }
        auto t = find(tableName, tables);

        validateWhereClause(deletionRoot->components[1], *t);

        // std::cout << "Deletion validated.\n\n";
    }

    // validate order clause
    void validateOrderClause(std::shared_ptr<node> orderRoot, const TableInfo& t) {

        if (orderRoot->type == nullnode)
            return;
        
        if (orderRoot->components[1]->type != kw_asc && orderRoot->components[1]->type != kw_desc) {
            std::cout << "Validator error. Somehow, ordering neither asc or desc.\n";
            exit(1);
        }

        // column name must not be in table.column form
        std::string columnName = orderRoot->components[0]->value;
        if (hasDot(columnName)) {
            std::cout << "Validator error. Ordered column \"" << t.name + '.' + columnName << "\" should not be in table.column form. Try \"" << split(columnName).second << "\".\n";
            exit(1);
        }

        // column must exist in the table
        if (!exists(columnName, t.columns)) {
            std::cout << "Validator error. Ordered column \"" << columnName << "\" does not exist in table \"" << t.name << "\".\n";
            exit(1);
        }
        auto c = find(columnName, t.columns);
        
        // column must not be a bool
        if (c->type == bool_literal) {
            std::cout << "Validator error. Cannot order by a boolean column \"" << columnName << "\".\n";
            exit(1);
        }
    }

    // validate selection
    void validateSelection(std::shared_ptr<node> selectionRoot) {

        // table must exist
        std::string tableName = selectionRoot->components[1]->value;
        if (!exists(tableName, tables)) {
            std::cout << "Validator error. Table \"" << tableName << "\" mentioned in update statement into does not exist.\n";
            exit(1);
        }
        auto t = find(tableName, tables);

        // asterisk column list
        std::shared_ptr<node> columnListRoot = selectionRoot->components[2];
        if (columnListRoot->components[0]->type == asterisk) {

        }

        // normal column list
        else {
            // columns must not be in table.column form
            for (auto& col : columnListRoot->components) {
                if (hasDot(col->value)) {
                    std::cout << "Validator error. Column \"" << col->value << "\" mentioned in selection should not be in table.column form. Try \"" << split(col->value).second << "\".\n";
                    exit(1);
                }
            }

            // @TODO add repeated with alias
            // cannot select the same column twice
            std::vector<std::string> colNames;
            for (auto& col : columnListRoot->components) {
                if (std::find(colNames.begin(), colNames.end(), col->value) != colNames.end()) {
                    std::cout << "Validator error. Attempted to select column \"" << col->value << "\" twice from table \"" << t->name << "\".\n";
                    exit(1); 
                }
                colNames.push_back(col->value);
            }
            
            // column must exist in table
            for (auto& c : columnListRoot->components) {
                if (!exists(c->value, t->columns)) {
                    std::cout << "Validator error. Selected column \"" << c->value << "\" does not exist in table \"" << t->name << "\".\n";
                    exit(1);
                }
            }
        }

        validateWhereClause(selectionRoot->components[3], *t);
        validateOrderClause(selectionRoot->components[4], *t);

        std::vector<ColumnInfo> workingColumns;
        // asterisk
        if (columnListRoot->components[0]->type == asterisk)
            workingColumns = t->columns;

        // else, add mentioned columns to working table
        else {
            for (const auto& selected : columnListRoot->components) {
                auto columnToAdd = std::find_if(t->columns.begin(), t->columns.end(), [&selected](auto& tc){return tc.name == selected->value;});
                workingColumns.push_back(ColumnInfo(*columnToAdd));
            }
        }

        workingTable.columns = workingColumns;

        // std::cout << "Selection validated.\n\n";
    }

    // validate boolean expression
    void validateBoolExpr(std::shared_ptr<node> boolExprRoot, const TableInfo& t) {
        
        // the following are validated by validating child bool_exprs
        // bool_expr bool_op bool_expr

        // (bool_expr)
        if (boolExprRoot->components.size() == 1 /*only child: boolExprRoot->components[0]->type == bool_expr*/) {
            // std::cout << "parens\n";
            validateBoolExpr(boolExprRoot->components[0], t);
            return;
        }
        // !(bool_expr)
        else if (boolExprRoot->components[0]->type == op_not) {
            // std::cout << "op not\n";
            validateBoolExpr(boolExprRoot->components[1], t);
            return;
        }
        // bool_expr bool_op bool_expr
        else if (boolExprRoot->components[1]->type == op_or || boolExprRoot->components[1]->type == op_and) {
            // std::cout << "bool op\n";
            validateBoolExpr(boolExprRoot->components[0], t);
            validateBoolExpr(boolExprRoot->components[2], t);
            return;
        }

        // the following are validated based on expression correctness

        // all of the following begin with identifiers and have 2 common checks
        // column names must not be in table.column form
        std::string lhsColumnName = boolExprRoot->components[0]->value;
        if (hasDot(lhsColumnName)) {
            std::cout << "Validator error. Column \"" << lhsColumnName << "\" should not be in table.column form. Try \"" << split(lhsColumnName).second << "\".\n";
            exit(1);   
        }
        // column must be in table
        if (!exists(lhsColumnName, t.columns)) {
            std::cout << "Validator error. Column \"" << lhsColumnName << "\" doesn't exist in table \"" << t.name << "\".\n";
            exit(1);
        }
        auto lhsColumn = find(lhsColumnName, t.columns);

        // identifier in identifier
        if (boolExprRoot->components[1]->type == kw_in) {
            // std::cout << "in\n";

            std::string rhsIdentifier = boolExprRoot->components[2]->value;
            if (!hasDot(rhsIdentifier)) {
                std::cout << "Validator error. Column \"" << rhsIdentifier << "\" in boolean expression must be in table.column form.\n";
                exit(1);   
            }

            // rhs table must exist
            std::string rhsColumnName = split(rhsIdentifier).second;
            std::string rhsTableName = split(rhsIdentifier).first;
            if (!exists(rhsTableName, tables)) {
                std::cout << "Validator error. Table \"" << rhsTableName << "\" mentioned in \"" << rhsIdentifier << "\" in boolean expression does not exist!\n";
                exit(1);   
            }
            auto rhsTable = find(rhsTableName, tables);

            // column must be in table
            if (!exists(rhsColumnName, rhsTable->columns)) {
                std::cout << "Validator error. Column \"" << rhsColumnName << "\" mentioned in boolean expression doesn't exist in table \"" << rhsTable->name << "\".\n";
                exit(1);
            }
            auto rhsColumn = find(rhsColumnName, rhsTable->columns);
            
            // check that lhs column and rhs column are the same type
            if (lhsColumn->type != rhsColumn->type) {
                std::cout << "Validator error. Types conflict in boolean expression when checking if " << tokenTypeToString(lhsColumn->type) << " \""
                          << t.name + '.' + lhsColumnName << "\" is in " << tokenTypeToString(rhsColumn->type) << " \"" << rhsIdentifier << "\".\n";
                exit(1);
            }
            
            return;
        }
        // identifier comparison any|all indentifier 
        else if (boolExprRoot->components[2]->type == kw_any || boolExprRoot->components[2]->type == kw_all) {
            // std::cout << "any/all\n";

            std::string rhsIdentifier = boolExprRoot->components[3]->value;
            if (!hasDot(rhsIdentifier)) {
                std::cout << "Validator error. Column \"" << rhsIdentifier << "\" in boolean expression must be in table.column form.\n";
                exit(1);   
            }

            // rhs table must exist
            std::string rhsColumnName = split(rhsIdentifier).second;
            std::string rhsTableName = split(rhsIdentifier).first;
            if (!exists(rhsTableName, tables)) {
                std::cout << "Validator error. Table \"" << rhsTableName << "\" mentioned in \"" << rhsIdentifier << "\" in boolean expression does not exist!\n";
                exit(1);   
            }
            auto rhsTable = find(rhsTableName, tables);

            // column must be in table
            if (!exists(rhsColumnName, rhsTable->columns)) {
                std::cout << "Validator error. Column \"" << rhsColumnName << "\" mentioned in boolean expression doesn't exist in table \"" << rhsTable->name << "\".\n";
                exit(1);
            }
            auto rhsColumn = find(rhsColumnName, rhsTable->columns);
            
            // check that lhs column and rhs column are the same type
            if (lhsColumn->type != rhsColumn->type) {
                std::cout << "Validator error. Types conflict in boolean expression when comparing " << tokenTypeToString(lhsColumn->type) << " \""
                          << t.name + '.' + lhsColumnName << "\" to all/any " << tokenTypeToString(rhsColumn->type) << " \"" << rhsIdentifier << "\".\n";
                exit(1);
            }

            // disallow <>= of bool columns
            element_type opType = boolExprRoot->components[1]->type;
            if (lhsColumn->type == bool_literal && (opType >= op_less_than && opType <= op_greater_than_equals)) {
                std::cout << "Validator error. Tried to use operator " << tokenTypeToString(opType)
                          << " with bool column \"" << t.name + '.' + lhsColumn->name << "\".\n";
                if (rhsColumn->type == bool_literal) {
                    std::cout << "Column \"" << rhsIdentifier << "\" is also of type bool.\n";
                }                          
                exit(1);
            }
            
            return;
        }
        // identifier comparison literal/identifier
        else {
            // std::cout << "simple comparison\n";

            // @TODO allow cross-type comparisons between ints and floats, disallow all others
            // type check, rhs literal must match lhs column type
            auto c = find(lhsColumnName, t.columns);
            element_type rhsType = boolExprRoot->components[2]->type;
            auto rhsValue = boolExprRoot->components[2]->value;

            if (rhsType != kw_null && rhsType != identifier) {  
                // @TODO can we guarantee that c->type is int, float chars, or bool literal?
                if (c->type != rhsType) {
                    std::cout << "Validator error. Type error in boolean expression between " << tokenTypeToString(c->type) << " column \"" 
                                << t.name + '.' + c->name << "\" and the attempted comparison to " << tokenTypeToString(rhsType) << ' ' << rhsValue << ".\n";
                    exit(1);
                }
            }
            
            // for rhs identifier
            if (rhsType == identifier) {
                // must not be in table.column form
                if (hasDot(rhsValue)) {
                    std::cout << "Validator error. Column \"" << rhsValue << "\" mentioned in a boolean expression should not be in table.column form.\n";
                    exit(1);
                }

                // column must exist in t
                if (!exists(rhsValue, t.columns)) {
                    std::cout << "Validator error. Column \"" << rhsValue << "\" does not exist in table \"" << t.name << "\".\n";
                    exit(1);    
                }
                auto rhsC = find(rhsValue, t.columns);

                // lhs and rhs columns must be same type
                if (c->type != rhsC->type) {
                    std::cout << "Validator error. Type conflict in boolean expression when comparing " << tokenTypeToString(c->type) << " \"" << c->name
                              << "\" to " << tokenTypeToString(rhsC->type) << " \"" << rhsValue << "\".\n.";
                    exit(1);
                }

                // disallow <>= on bool columns
                element_type opType = boolExprRoot->components[1]->type;
                if (lhsColumn->type == bool_literal && (opType >= op_less_than && opType <= op_greater_than_equals)) {
                    std::cout << "Validator error. Tried to use operator " << tokenTypeToString(opType)
                            << " with bool column \"" << t.name + '.' + lhsColumn->name << "\".\n";
                    if (rhsC->type == bool_literal) {
                        std::cout << "Column \"" << rhsValue << "\" is also of type bool.\n";
                    }                          
                    exit(1);
                }
            }

            return;
        }
    }

    // validate where clause
    void validateWhereClause(std::shared_ptr<node> whereClauseRoot, const TableInfo& t) {

        if (whereClauseRoot->type == nullnode)
            return;

        validateBoolExpr(whereClauseRoot->components[0], t);
    }

    // validate update
    void validateUpdate(std::shared_ptr<node> updateRoot) {
        auto columnValueListRoot = updateRoot->components[1];

        // table must exist
        std::string tableName = updateRoot->components[0]->value;
        if (!exists(tableName, tables)) {
            std::cout << "Validator error. Table \"" << tableName << "\" mentioned in update statement into does not exist.\n";
            exit(1);
        }

        // columns must not be in table.column form
        for (auto& columnValuePair : columnValueListRoot->components) {
            if (hasDot(columnValuePair->components[0]->value)) {
                std::cout << "Validator error. Column \"" << columnValuePair->components[0]->value << "\" mentioned in update statement should not be in table.column form. Try \"" << split(columnValuePair->components[0]->value).second << "\".\n";
                exit(1);
            }
        }

        // columns in column-value list must exist in table
        auto t = find(tableName, tables);
        for (auto& columnValuePair : columnValueListRoot->components) {
            if (!exists(columnValuePair->components[0]->value , t->columns)) {
                std::cout << "Validator error. Column \"" << columnValuePair->components[0]->value << "\" does not exist in table \"" << t->name << "\".\n";
                exit(1);     
            }
        }

        // cannot update a value of the wrong type of the column
        for (auto& columnValuePair : columnValueListRoot->components) {
            auto c = find(columnValuePair->components[0]->value, t->columns);
            element_type pairType = columnValuePair->components[1]->type;
            std::string pairValue = columnValuePair->components[1]->value;

            // a null can be updateed in any column
            if (pairType == kw_null)
                continue;
            
            // @TODO can we guarantee that c->type is int, float chars, or bool literal?
            // ex: if the node is an int literal, the column type must also be an int literal
            if (c->type != pairType) {
                std::cout << "Validator error. Column \"" << t->name + '.' + c->name << "\" is of type " << tokenTypeToString(c->type) << ", but an update of "
                        << tokenTypeToString(pairType) << " " << pairValue << " was attempted.\n";
                exit(1);
            }
        }

        // cannot update the same column twice
        std::vector<std::string> colNames;
        for (auto& columnValuePair : columnValueListRoot->components) {
            if (std::find(colNames.begin(), colNames.end(), columnValuePair->components[0]->value) != colNames.end()) {
                std::cout << "Validator error. There were two updates of column \"" << t->name + '.' + columnValuePair->components[0]->value << "\" within the same statement.\n";
                exit(1); 
            }
            colNames.push_back(columnValuePair->components[0]->value);
        }

        // cannot update more chars than the max length for that column
        for (auto& columnValuePair : columnValueListRoot->components) {
            auto c = find(columnValuePair->components[0]->value, t->columns);
            if (columnValuePair->components[1]->type == chars_literal && columnValuePair->components[1]->value.length() > c->charsLength) {
                std::cout << "Validator error. The maximum string length of \"" <<  t->name + '.' + columnValuePair->components[0]->value << "\" is " << c->charsLength << " character(s).\n";
                exit(1);  
            }
        }

        // @NOTE ignore unmentioned columns.

        // WHERE CLAUSE
        auto whereClauseRoot = updateRoot->components[2];
        validateWhereClause(updateRoot->components[2], *t);
        
        // std::cout << "Update validated.\n\n";
    }

    // validate insertion
    void validateInsertion(std::shared_ptr<node> insertionRoot) {
        auto columnValueListRoot = insertionRoot->components[1];

        // table must exist
        std::string tableName = insertionRoot->components[0]->value;
        if (!exists(tableName, tables)) {
            std::cout << "Validator error. Table \"" << tableName << "\" mentioned in insert statement does not exist.\n";
            exit(1);
        }

        // columns must not be in table.column form
        for (auto& columnValuePair : columnValueListRoot->components) {
            if (hasDot(columnValuePair->components[0]->value)) {
                std::cout << "Validator error. Column \"" << columnValuePair->components[0]->value << "\" mentioned in insert statement should not be in table.column form. Try \"" << split(columnValuePair->components[0]->value).second << "\".\n";
                exit(1);
            }
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
        for (auto& columnValuePair : columnValueListRoot->components) {
            auto c = find(columnValuePair->components[0]->value, t->columns);
            element_type pairType = columnValuePair->components[1]->type;
            std::string pairValue = columnValuePair->components[1]->value;

            // a null can be updateed in any column
            if (pairType == kw_null)
                continue;
            
            // @TODO can we guarantee that c->type is int, float chars, or bool literal?
            // ex: if the node is an int literal, the column type must also be an int literal
            if (c->type != pairType) {
                std::cout << "Validator error. Column \"" << t->name + '.' + c->name << "\" is of type " << tokenTypeToString(c->type) 
                          << ", but an insert of " << tokenTypeToString(pairType) << " " << pairValue << " was attempted.\n";
                exit(1);
            }
        }

        // cannot insert into the same column twice
        std::vector<std::string> colNames;
        for (auto& columnValuePair : columnValueListRoot->components) {
            if (std::find(colNames.begin(), colNames.end(), columnValuePair->components[0]->value) != colNames.end()) {
                std::cout << "Validator error. There were two insertions into column \"" << t->name + '.' + columnValuePair->components[0]->value << "\" within the same statement.\n";
                exit(1); 
            }
            colNames.push_back(columnValuePair->components[0]->value);
        }

        // cannot insert more chars than the max length for that column
        for (auto& columnValuePair : columnValueListRoot->components) {
            auto c = find(columnValuePair->components[0]->value, t->columns);
            if (columnValuePair->components[1]->type == chars_literal && columnValuePair->components[1]->value.length() > c->charsLength) {
                std::cout << "Validator error. The maximum string length of \"" <<  t->name + '.' + columnValuePair->components[0]->value << "\" is " << c->charsLength << " character(s).\n";
                exit(1);  
            }
        }

        // @NOTE unmentioned columns assume null insert.

        // std::cout << "Insert validated.\n\n";
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
        // verify that the joined columns exist in their respective tables
        auto joinedColumn1 = std::find_if(table1->columns.begin(), table1->columns.end(), [&split1](const auto& c){return c.name == split1.second;});
        if (joinedColumn1 == table1->columns.end()) {
            std::cout << "Validator error. Column \"" << split1.second << "\" isn't a column in table \"" << table1->name << "\".\n";
            exit(1);
        }
        auto joinedColumn2 = std::find_if(table2->columns.begin(), table2->columns.end(), [&split2](const auto& c){return c.name == split2.second;});
        if (joinedColumn2 == table2->columns.end()) {
            std::cout << "Validator error. Column \"" << split2.second << "\" isn't a column in table \"" << table2->name << "\".\n";
            exit(1);
        }

        // joined columns must be of the same type
        if (joinedColumn1->type != joinedColumn2->type) {
            std::cout << "Validator error. Column \"" << col1Name << "\" is type " << joinedColumn1->type << ", and \"" << col2Name << "\" is type " << joinedColumn2->type << ".\n";
            exit(1);
        }

        // disallow <>= on bool columns in on expr
        element_type opType = onExprRoot->components[1]->type;
        if (joinedColumn1->type == bool_literal && (opType >= op_less_than && opType <= op_greater_than_equals)) {
            std::cout << "Validator error. Attempted to join on two bool columns \"" << col1Name << "\" and \"" << col2Name << "\", but the comparison is neither '==' nor '!='.\n";                     
            exit(1);
        }

        // @NOTE: if there are vestiges of joined column alias in on_expr, remove them.

        // ALIAS LIST
        std::shared_ptr<node> aliasListRoot = joinRoot->components[3];

        // require aliasing on column name conflicts
        std::vector<std::string> conflictingColumnNames;
        // identify all conflicting names
        for (auto& col : table1->columns) {
            if (exists(col.name, table2->columns)) 
                conflictingColumnNames.push_back(col.name);
        }
        // if there is at least one name conflict, make sure alias list is not nullnode
        if (conflictingColumnNames.size() != 0 && aliasListRoot->type == nullnode) {
            std::cout << "Validator error. There are name conflicts in a join, but no alias list.\n";
            exit(1);
        }
        // for each conflicting name, check that table1.name or table2.name is aliased
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

        // all columns to alias must be in table.column form
        for (auto& aliasRoot : aliasListRoot->components) {
            if(!hasDot(aliasRoot->components[0]->value)) {
                std::cout << "Validator error. Aliased column \"" << aliasRoot->components[0]->value << "\" is not in table.column form.\n";
                exit(1);
            }
        }

        // all aliases must NOT be in table.column form
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

        // @TODO the below approaches disallow using names that have been aliased away
        // e.g. if t1.x is aliased to x1, t2.y cannot be aliased to x
        // allow that kind of thing (low-priority, this could cause headaches anyways)

        // alias names must not conflict with eachother or columns in table1 or table2
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
        }

        // resultant columns to workingTable
        std::vector<ColumnInfo> workingColumns;

        // build map from table.column name to alias
        std::map<std::string, std::string> nameToAlias;
        for (auto& aliasRoot : aliasListRoot->components)
            nameToAlias.insert({aliasRoot->components[0]->value, aliasRoot->components[1]->value});

        // go through columns of each table
        for (const auto& t : {table1, table2}) {
            for (const ColumnInfo& c : t->columns) {
                std::string aliasName;                
                // no alias, add original name
                if (std::find_if(nameToAlias.begin(), nameToAlias.end(), [&t, &c, &aliasName](std::pair<std::string, std::string> nameToAlias){aliasName = nameToAlias.second; return nameToAlias.first == t->name + '.' + c.name;}) == nameToAlias.end())
                    workingColumns.push_back(ColumnInfo(c));
                // otherwise, add the alias as the name instead
                else
                    workingColumns.push_back(ColumnInfo(aliasName, c.type, c.charsLength));
            }
        }
        workingTable.columns = workingColumns;

        // std::cout << "Join validated.\n\n";
    }

    // validate bag operation
    void validateBagOp(std::shared_ptr<node> bagOpRoot) {

        // only union/intersect tables that exist
        std::string table1Name = bagOpRoot->components[1]->value;
        std::string table2Name = bagOpRoot->components[2]->value;
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
        for (const auto& c : first->columns) {
            if (!exists(c.name, second->columns)) {
                std::cout << "Validator error. Tables \"" << table1Name << "\" and \"" << table2Name << "\" have different column names.\n";
                exit(1);
            }
        }

        // cannot union|intersect tables with different column types
        for (const auto& c : first->columns) {
            auto c2 = find(c.name, second->columns);
            if (c.type != c2->type) {
                std::cout << "Validator error. Tables \"" << table1Name << "\" and \"" << table2Name << "\" have different column types.\n";
                exit(1);
            }
        }

        // @TODO default to larger chars on resultant table
        std::vector<ColumnInfo> workingColumns = first->columns;
        // for each chars column, make sure that the larger one is put into workingColumns  
        for (auto& workingColumn : workingColumns) {
            if (workingColumn.type == chars_literal) {
                auto c2 = find(workingColumn.name, second->columns);
                workingColumn.charsLength  = ( c2->charsLength > workingColumn.charsLength ? c2->charsLength : workingColumn.charsLength );
            }
        }

        workingTable.columns = workingColumns;

        // std::cout << "Bag operation validated.\n\n";
    }

    // validate definition
    void validateDefinition(std::shared_ptr<node> definitionRoot) {

        // if table already exists
        std::string tableName = definitionRoot->components[1]->value;
        if (exists(tableName, tables)) {
            std::cout << "Validator error. Table \"" << tableName << "\" already exists. Cannot define a table with the same name.\n";
            exit(1);
        }

        // bag the working table name to the new table's name
        workingTable.name = tableName;

        // defined selection
        if (definitionRoot->components[2]->type == selection) {
            validateSelection(definitionRoot->components[2]);
            tables.push_back(workingTable);
        }

        // defined bag_op
        else if (definitionRoot->components[2]->type == bag_op) {
            validateBagOp(definitionRoot->components[2]);
            tables.push_back(workingTable);
        }

        // defined join
        else if (definitionRoot->components[2]->type == join) {
            validateJoin(definitionRoot->components[2]);
            tables.push_back(workingTable);
        }

        // defined column, type list
        else if (definitionRoot->components[2]->type == col_type_list) {
            // chars must have length > 0, length < 256
            for (auto columnTypePair : definitionRoot->components[2]->components) {
                if (columnTypePair->components[1]->type == kw_chars) {
                    if (stoi(columnTypePair->components[2]->value) <= 0) {
                        std::cout << "Validator error. Column \"" << columnTypePair->components[0]->value << "\" in defined table \"" << definitionRoot->components[1]->value << "\" may not have a non-positive number of characters.\n";
                        exit(1);
                    }
                    if (stoi(columnTypePair->components[2]->value) > 255) {
                        std::cout << "Validator error. Column \"" << columnTypePair->components[0]->value << "\" in defined table \"" << definitionRoot->components[1]->value << "\" may not have more than 255 characters.\n";
                        exit(1);
                    }
                }
            }

            // column names must NOT be in table.column form
            for (auto columnTypePair : definitionRoot->components[2]->components) {
                std::string colName = columnTypePair->components[0]->value;
                if (hasDot(colName)) {
                    std::cout << "Validator error. Attempted to define table \"" << tableName << "\", but column \"" << colName << "\" has a dot. Try \"" << split(colName).second << "\".\n";
                    exit(1);
                }
            }

            // cannot create a table with two columns of the same name
            std::vector<std::string> names;
            for (auto columnTypePair : definitionRoot->components[2]->components) {
                if (std::find(names.begin(), names.end(), columnTypePair->components[0]->value) != names.end()) {
                    std::cout << "Validator error. More than one definition of column \"" << columnTypePair->components[0]->value << "\" in definition of table \"" << tableName << "\".\n";
                    exit(1);
                }
                names.push_back(columnTypePair->components[0]->value);
            }
            
            tables.push_back(nodeToTableInfo(definitionRoot));
        }

        // std::cout << "Definition validated.\n";
        // printTableList(tables);
        // std::cout << '\n';
    }

    // validate drop statement
    void validateDrop(std::shared_ptr<node> dropRoot) {
        std::string tableName = dropRoot->components[0]->value;

        // if table exists, drop it
        if (exists(tableName, tables)) 
            tables.erase(find(tableName, tables));

        // if table is in neither, error
        else {
            std::cout << "Validator error. Table \"" << tableName << "\" doesn't exist.\n";
            exit(1);
        }

        // std::cout << "Drop statement validated.\n";
        // printTableList(tables);
        // std::cout << '\n';
        return;
    }
    
};

#endif