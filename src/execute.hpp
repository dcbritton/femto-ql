// execute.hpp

#ifndef EXECUTE
#define EXECUTE

#include <fstream>
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <string>
#include <map>
#include "TableInfo.hpp"
#include "node.hpp"
#include "Table.hpp"
#include "convert.hpp"

// mark a row for deletion
void executeDeletion(std::shared_ptr<node> deletionRoot) {
    std::string tableName = deletionRoot->components[0]->value;
    TableInfo t(DIRECTORY + tableName + FILE_EXTENSION);
    auto boolExprRoot = deletionRoot->components[1]->components[0];

    Table table(t);
    std::shared_ptr<EvaluationNode> EvaluationRoot = convert(boolExprRoot, table, t);
    while (table.nextRow()) {
        if (!EvaluationRoot->evaluate())
            continue;
        table.markForDeletion();
    }
}

// execute bag union/intersect
void executeBagOp(std::shared_ptr<node> selectionRoot) {
    std::string table1Name = selectionRoot->components[1]->value;
    std::string table2Name = selectionRoot->components[2]->value;
    TableInfo t1(DIRECTORY + table1Name + FILE_EXTENSION);
    TableInfo t2(DIRECTORY + table2Name + FILE_EXTENSION);
    element_type bagOpType = selectionRoot->components[0]->type;

    // statement output
    std::cout << "$ bag " << (bagOpType == kw_union ? "union " : "intersect ") << table1Name << ", " << table2Name << '\n';

    Table table1(t1);
    Table table2(t2);

    // get a vector of pointers to ColumnInfos with the larger widths
    std::vector<const ColumnInfo*> largerColumns;
    for (const ColumnInfo& column : t1.columns)
        largerColumns.push_back(column.outputWidth > t2[column.name]->outputWidth ? &column : t2[column.name]);

    // output column names
    std::cout << UNDERLINE;
    for (const ColumnInfo* column : largerColumns)
        std::cout << UNDERLINE << std::right << std::setw(column->outputWidth) << column->name << ' ';
    std::cout << '\n' << CLOSEUNDERLINE;

    // bag union
    if (bagOpType == kw_union) {
        // output first table
        while (table1.nextRow()) {
            for (const ColumnInfo* column : largerColumns)
                std::cout << std::right << std::setw(column->outputWidth) << table1.getValueString(column->name) << ' ';
            std::cout << '\n';
        }
        // output second table
        while (table2.nextRow()) {
            for (const ColumnInfo* column : largerColumns)
                std::cout << std::right << std::setw(column->outputWidth) << table2.getValueString(column->name) << ' ';
            std::cout << '\n';
        }
    }

    // bag intersect
    else if (bagOpType == kw_intersect)  {
        while (table1.nextRow()) {
            // compare each row of table1 to every row of table2 until a match is found
            while (table2.nextRow()) {
                if (!table1.compareRow(table2))
                    continue;
                // match found, output, reset table 2
                for (const ColumnInfo* column : largerColumns)
                    std::cout << std::right << std::setw(column->outputWidth) << table1.getValueString(column->name) << ' ';
                std::cout << '\n';
                table2.reset();
                break;
            }
            // match never found, reset table2
            table2.reset();
        }
    }
    std::cout << '\n';
}

// execute selection
void select(std::shared_ptr<node> selectionRoot) {
    std::string tableName = selectionRoot->components[1]->value;
    TableInfo t(DIRECTORY + tableName + FILE_EXTENSION);

    // get a list of all column names to select
    std::vector<std::string> selectedColumnNames;
    auto columnList = selectionRoot->components[2];
    // * column list
    if (columnList->components[0]->type == asterisk)
        for (auto& column : t.columns)
            selectedColumnNames.push_back(column.name);
    // column list with names
    else
        for (auto& selectedColumnNode : columnList->components)
            selectedColumnNames.push_back(selectedColumnNode->value);

    // @TODO output formatting

    // output statement
    std::cout << "$ select from " << tableName << "...\n";

    // output selected column names
    for (auto& name : selectedColumnNames)
        std::cout << name << ' ';
    std::cout << '\n';

    Table table(t);

    // no where clause
    auto whereClauseRoot = selectionRoot->components[3];
    if (whereClauseRoot->type == nullnode) {
        while (table.nextRow()) {
            table.printRow(std::cout, selectedColumnNames);
        }
        return;
    }
    // where clause
    auto boolExprRoot = whereClauseRoot->components[0];
    std::shared_ptr<EvaluationNode> evaluationRoot = convert(boolExprRoot, table, t);
    // @TODO evaluationRoot->bind(eIt);
    while (table.nextRow()) {
        if (!evaluationRoot->evaluate())
            continue;
        table.printRow(std::cout, selectedColumnNames);
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
    TableInfo t(DIRECTORY + tableName + FILE_EXTENSION);
    auto ColumnValueList = updateRoot->components[1];

    // build list of columns and values to write
    std::map<std::string, WriteData> mentionedNameToWriteData;
    for (auto& columnValuePair : ColumnValueList->components )
        // insert(name, WriteData{value, type})                                          
        mentionedNameToWriteData.insert({columnValuePair->components[0]->value, WriteData{columnValuePair->components[1]->value, columnValuePair->components[1]->type}});   

    Table table(t);
    auto boolExprRoot = updateRoot->components[2];
    std::shared_ptr<EvaluationNode> evaluationRoot =  convert(boolExprRoot->components[0], table, t);
    while (table.nextRow()) {
        if(!evaluationRoot->evaluate())
            continue;
       
        for (auto& entry : mentionedNameToWriteData) {
            switch (entry.second.type) {
                case int_literal:
                    table.setInt(entry.first, stoi(entry.second.value));
                    break;
                case float_literal:
                    table.setFloat(entry.first, stof(entry.second.value));
                    break;
                case chars_literal:
                    table.setChars(entry.first, entry.second.value);
                    break;
                case bool_literal:
                    table.setBool(entry.first, entry.second.value == "true");
                    break;
                case kw_null:
                    table.setNull(entry.first);
                    break;
                default:
                    std::cout << "Error while executing an update. Column cannot be a type other than a literal.\n";
                    exit(1);
            }
        }
    }
}

// used in insert to write a value given a string from the AST
void writeValue(const std::string& value, const ColumnInfo& c, std::ofstream& file) {
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

    TableInfo t(DIRECTORY + tableName + FILE_EXTENSION);
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
    
    TableInfo table = nodeToTableInfo(definitionRoot);
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

            case deletion:
                executeDeletion(statementRoot);
                break;

            case update:
                executeUpdate(statementRoot);
                break;
            
            case bag_op:
                executeBagOp(statementRoot);
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