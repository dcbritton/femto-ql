// execute.hpp

#ifndef EXECUTE
#define EXECUTE

#include <fstream>
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <string>
#include <map>
#include "table.hpp"
#include "node.hpp"
#include "RowIterator.hpp"
#include "convert.hpp"

// execute union/intersect
void executeSetOp(std::shared_ptr<node> selectionRoot) {
    std::string table1Name = selectionRoot->components[1]->value;
    std::string table2Name = selectionRoot->components[2]->value;
    table t1(DIRECTORY + table1Name + FILE_EXTENSION);
    table t2(DIRECTORY + table2Name + FILE_EXTENSION);

    for (column& c : t1.columns)
        std::cout << c.name << ' ';
    std::cout << '\n';

    // @TODO unions should not repeat rows
    if (selectionRoot->components[0]->type == kw_union) {
        RowIterator t1Iterator(t1);
        RowIterator t2Iterator(t2);
        while (t1Iterator.next()) {
            for (column& c : t1.columns) {
                std::cout << t1Iterator.getValueString(c.name) << ' ';
            }
            std::cout << '\n';
        }
        while (t2Iterator.next()) {
            // t1.columns below is NOT a mistake
            // t2 and t1 are verified to have the same column names
            // this is a trick to get column order correct
            for (column& c : t1.columns) {
                std::cout << t2Iterator.getValueString(c.name) << ' ';
            }
            std::cout << '\n';
        }
    }

    // intersect
    else if (selectionRoot->components[0]->type == kw_intersect)  {
        RowIterator t1Iterator(t1);
        RowIterator t2Iterator(t2);


    }
    std::cout << '\n';
}

// execute selection
void select(std::shared_ptr<node> selectionRoot) {
    std::string tableName = selectionRoot->components[1]->value;
    table t(DIRECTORY + tableName + FILE_EXTENSION);

    // get a list of all column names to select
    std::vector<std::string> mentionedColumns;
    auto columnList = selectionRoot->components[2];
    // * column list
    if (columnList->components[0]->type == asterisk)
        for (auto& c : t.columns)
            mentionedColumns.push_back(c.name);
    // column list with names
    else
        for (auto& mentionedColumn : columnList->components)
            mentionedColumns.push_back(mentionedColumn->value);

    // @TODO output formatting

    // output
    for (auto& name : mentionedColumns)
        std::cout << name << ' ';
    std::cout << '\n';

    RowIterator rowIt(t);

    // no where clause
    auto whereClauseRoot = selectionRoot->components[3];
    if (whereClauseRoot->type == nullnode) {
        while (rowIt.next()) {
            for (std::string& name : mentionedColumns) {
                    std::cout << rowIt.getValueString(name) << ' ';
            }
            std::cout << '\n';
        }
        return;
    }
    // where clause
    auto boolExprRoot = whereClauseRoot->components[0];
    std::shared_ptr<EvaluationNode> evaluationRoot = convert(boolExprRoot, rowIt, t);
    // @TODO evaluationRoot->bind(eIt);
    while (rowIt.next()) {
        if (!evaluationRoot->evaluate())
            continue;
            
        for (std::string& name : mentionedColumns) {
            std::cout << rowIt.getValueString(name) << ' ';
        }
        std::cout << '\n';
    }
    std::cout << '\n';
}

// used in executeUpdate()
struct WriteData {
    std::string& value;
    element_type type;
};

void executeUpdate(std::shared_ptr<node> updateRoot) {
    std::string tableName = updateRoot->components[0]->value;
    table t(DIRECTORY + tableName + FILE_EXTENSION);
    auto ColumnValueList = updateRoot->components[1];

    // build list of columns and values to write
    std::map<std::string, WriteData> mentionedNameToWriteData;
    for (auto& columnValuePair : ColumnValueList->components )
        // insert(name, WriteData{value, type})                                          
        mentionedNameToWriteData.insert({columnValuePair->components[0]->value, WriteData{columnValuePair->components[1]->value, columnValuePair->components[1]->type}});   

    RowIterator rowIt(t);
    auto boolExprRoot = updateRoot->components[2];
    std::shared_ptr<EvaluationNode> evaluationRoot =  convert(boolExprRoot->components[0], rowIt, t);
    while (rowIt.next()) {
        if(!evaluationRoot->evaluate())
            continue;
       
        for (auto& entry : mentionedNameToWriteData) {
            switch (entry.second.type) {
                case int_literal:
                    rowIt.setInt(entry.first, stoi(entry.second.value));
                    break;
                case float_literal:
                    rowIt.setFloat(entry.first, stof(entry.second.value));
                    break;
                case chars_literal:
                    rowIt.setChars(entry.first, entry.second.value);
                    break;
                case bool_literal:
                    rowIt.setBool(entry.first, entry.second.value == "true");
                    break;
                case kw_null:
                    rowIt.setNull(entry.first);
                    break;
                default:
                    std::cout << "Error while executing an update. Column cannot be a type other than a literal.\n";
                    exit(1);
            }
        }
    }
}

// used in insert to write a value given a string from the AST
void writeValue(const std::string& value, const column& c, std::ofstream& file) {
    switch (c.type) {
        case int_literal: {
            int inum = stoi(value);
            file.write(reinterpret_cast<const char*>(&inum), sizeof(inum));
            break;
        }
        case float_literal: {
            float fnum = stof(value);
            file.write(reinterpret_cast<const char*>(&fnum), sizeof(fnum));
            break;
        }
        case chars_literal: {
            file.write(value.c_str(), value.length());
            for (int i = 0; i < c.charsLength-value.length(); ++i) {
                file << '\0';
            }
            break;
        }
        case bool_literal: {
            file << static_cast<unsigned char>(value == "true" ? 0b1 : 0b0 );
            break;
        }
    }
}

void insert(std::shared_ptr<node> insertRoot) {

    std::string tableName = insertRoot->components[0]->value;
    std::ofstream file(DIRECTORY + tableName + FILE_EXTENSION, std::ios_base::app);

    // write the 'delete' byte as 0
    file << static_cast<unsigned char>(0b0);

    auto columnValueListRoot = insertRoot->components[1];

    table t(DIRECTORY + tableName + FILE_EXTENSION);
    for (auto& c : t.columns) {
        bool mentioned = false;
        for (auto& columnValuePair : columnValueListRoot->components) {
            // @TODO mentioned & null insert have the same logic. find a way to simplify & denest
            int numBytesToWrite = 4;
            // column mentioned
            if (columnValuePair->components[0]->value == c.name) {
                mentioned = true;
                // null insert, write null byte as 1, fill rest as 0
                if (columnValuePair->components[1]->type == kw_null) {
                        file << static_cast<unsigned char>(0b1);
                    if (c.type == chars_literal)
                        numBytesToWrite = c.charsLength;
                    if (c.type == bool_literal)
                        numBytesToWrite = 1;
                    for (int i = 0; i < numBytesToWrite; ++i)
                        file << static_cast<unsigned char>(0b0);
                }
                // value to insert
                else {
                    file << static_cast<unsigned char>(0b0);
                    writeValue(columnValuePair->components[1]->value, c, file);
                }
            }
        }
        // otherwise, write null byte as 1, fill rest as 0
        if (!mentioned) {
            std::cout << "Unmentioned: " << c.name;
            file << static_cast<unsigned char>(0b1);
            int numBytesToWrite = 4;
            if (c.type == chars_literal)
                numBytesToWrite = c.charsLength;
            if (c.type == bool_literal)
                numBytesToWrite = 1;
            for (int i = 0; i < numBytesToWrite; ++i)
                file << static_cast<unsigned char>(0b0);
        }
    }
}

void executeDrop(std::shared_ptr<node> dropRoot)  {
    std::filesystem::remove("../tables/" + dropRoot->components[0]->value + ".ftbl");
}

void define(std::shared_ptr<node> definitionRoot) {
    
    table table = nodeToTable(definitionRoot);
    std::ofstream header(DIRECTORY + table.name + FILE_EXTENSION);

    // bytes 0-63 for tableName
    header.write((table.name + std::string(64-table.name.length(), '\0')).c_str(), 64);

    // next 4 bytes reserved for # of columns
    int numColumns = table.columns.size();
    header.write(reinterpret_cast<const char*>(&numColumns), sizeof(int));

    // 64 bytes for column name followed by 4 bytes for type
    for (const auto& col : table.columns) {
        header.write((col.name + std::string(64-col.name.length(), '\0')).c_str(), 64);

        // first byte for type
        header << columnTypeToByte(col.type);

        // next two pad, last one gives number for chars, NUL for non-chars
        header << '\0' << '\0';
        header << static_cast<unsigned char>(col.type == chars_literal ? col.charsLength : 0 );
    }

    header.close();
}

void execute(std::shared_ptr<node> scriptRoot) {
    for (auto& statementRoot: scriptRoot->components) {
        switch (statementRoot->type) {

            case update:
                executeUpdate(statementRoot);
                break;
            
            case set_op:
                executeSetOp(statementRoot);
                break;

            case selection:
                select(statementRoot);
                break;

            case definition:
                define(statementRoot);
                break;

            case insertion:
                insert(statementRoot);
                break;
            
            case drop:
                executeDrop(statementRoot);
                break;

            default:
                std::cout << "Unknown statement type to execute.\n";
        }
    }
}

#endif