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

#define UNDERLINE "\033[4m"
#define CLOSEUNDERLINE "\033[0m"

// join
void executeJoin(std::shared_ptr<node> joinRoot) {
    std::string table1Name = joinRoot->components[0]->value;
    std::string table2Name = joinRoot->components[1]->value;
    TableInfo t1(TABLE_DIRECTORY + table1Name + FILE_EXTENSION);
    TableInfo t2(TABLE_DIRECTORY + table2Name + FILE_EXTENSION);

    auto onRoot = joinRoot->components[2];
    auto aliasListRoot = joinRoot->components[3];
    auto joinedColumn1Name = split(onRoot->components[0]->value);
    auto joinedColumn2Name = split(onRoot->components[2]->value);
    element_type operation = onRoot->components[1]->type;

    Table table1(t1);
    Table table2(t2);

    // output statment
    std::cout << "\n\033[0;34m$ join\033[0m " << "\033[0;32m" << table1Name << ", " << table2Name << "\033[0m" << '\n';

    // output column names
    std::vector<ColumnInfo> columns;
    // same as validation to find column names
    // build map from table.column name to alias
    std::map<std::string, std::string> nameToAlias;
    for (auto& aliasRoot : aliasListRoot->components)
        nameToAlias.insert({aliasRoot->components[0]->value, aliasRoot->components[1]->value});
    // go through columns of each table
    for (const auto& t : {&t1, &t2}) {
        for (const ColumnInfo& c : t->columns) {
            std::string aliasName;                
            // no alias, add original name
            if (std::find_if(nameToAlias.begin(), nameToAlias.end(), [&t, &c, &aliasName](std::pair<std::string, std::string> nameToAlias){aliasName = nameToAlias.second; return nameToAlias.first == t->name + '.' + c.name;}) == nameToAlias.end())
                columns.push_back(ColumnInfo(c));
            // otherwise, add the alias as the name instead
            else
                columns.push_back(ColumnInfo(aliasName, c.type, c.charsLength));
        }
    }
    
    std::cout << UNDERLINE;
    for (auto& column : columns) {
        std::cout << std::right << std::setw(column.outputWidth) << column.name << ' ';
    }
    std::cout << '\n' << CLOSEUNDERLINE;

    // output the join
    while (table1.nextRow()) {
        while (table2.nextRow()) {
            // match not found, skip
            if (!table1.compareCell(joinedColumn1Name.second, operation, table2, joinedColumn2Name.second))
                continue;

            // match found, output row
            for (const ColumnInfo column : t1.columns)
                std::cout << std::right << std::setw(column.outputWidth) << table1.getValueString(column.name) << ' ';
            for (const ColumnInfo column : t2.columns)
                std::cout << std::right << std::setw(column.outputWidth) << table2.getValueString(column.name) << ' ';
            
            // dont break, continue looking for matches
            std::cout << '\n';
        }
        // reached the end of table 2, reset
        table2.reset();
    }
}

// mark a row for deletion
void executeDeletion(std::shared_ptr<node> deletionRoot) {
    std::string tableName = deletionRoot->components[0]->value;
    TableInfo t(TABLE_DIRECTORY + tableName + FILE_EXTENSION);
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
    TableInfo t1(TABLE_DIRECTORY + table1Name + FILE_EXTENSION);
    TableInfo t2(TABLE_DIRECTORY + table2Name + FILE_EXTENSION);
    element_type bagOpType = selectionRoot->components[0]->type;

    // statement output
    std::cout << "\n\033[0;34m$ bag " << (bagOpType == kw_union ? "union\033[0m " : "intersect\033[0m ") << "\033[0;32m" << table1Name << ", " << table2Name << "\033[0m" << '\n';

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
        // output first table using larger output width
        while (table1.nextRow()) {
            for (const ColumnInfo* column : largerColumns)
                std::cout << std::right << std::setw(column->outputWidth) << table1.getValueString(column->name) << ' ';
            std::cout << '\n';
        }
        // output second table using larger output width
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
                // match not found, skip
                if (!table1.compareRow(table2))
                    continue;

                // match found. output & reset table2
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
}

// execute selection
void select(std::shared_ptr<node> selectionRoot) {
    std::string tableName = selectionRoot->components[1]->value;
    TableInfo t(TABLE_DIRECTORY + tableName + FILE_EXTENSION);

    // get a list of all column names to select
    std::vector<const ColumnInfo*> selectedColumns;
    auto columnList = selectionRoot->components[2];
    // * column list
    if (columnList->components[0]->type == asterisk)
        for (auto& column : t.columns)
            selectedColumns.push_back(&column);
    // column list with names
    else
        for (auto& selectedColumnNode : columnList->components)
            selectedColumns.push_back(t[selectedColumnNode->value]);

    // output statement
    std::cout << "\n\033[0;34m$ select from \033[0;32m" << tableName << "\033[0m" << '\n';

    // output column names
    std::cout << UNDERLINE;
    for (const ColumnInfo* column : selectedColumns)
        std::cout << UNDERLINE << std::right << std::setw(column->outputWidth) << column->name << ' ';
    std::cout << '\n' << CLOSEUNDERLINE;

    Table table(t);

    // no where clause
    auto whereClauseRoot = selectionRoot->components[3];
    if (whereClauseRoot->type == nullnode) {
        while (table.nextRow()) {
            for (const ColumnInfo* column : selectedColumns)
                std::cout << std::right << std::setw(column->outputWidth) << table.getValueString(column->name) << ' ';
            std::cout << '\n';
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
        for (const ColumnInfo* column : selectedColumns)
            std::cout << std::right << std::setw(column->outputWidth) << table.getValueString(column->name) << ' ';
        std::cout << '\n';
    }

}

// used in executeUpdate()
struct WriteData {
    std::string& value;
    element_type type;
};

// update an entry
void executeUpdate(std::shared_ptr<node> updateRoot) {
    std::string tableName = updateRoot->components[0]->value;
    TableInfo t(TABLE_DIRECTORY + tableName + FILE_EXTENSION);
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
       
        for (const auto& [name, data] : mentionedNameToWriteData) {
            switch (data.type) {
                case int_literal:
                    table.setInt(name, stoi(data.value));
                    break;
                case float_literal:
                    table.setFloat(name, stof(data.value));
                    break;
                case chars_literal:
                    table.setChars(name, data.value);
                    break;
                case bool_literal:
                    table.setBool(name, data.value == "true");
                    break;
                case kw_null:
                    table.setNull(name);
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

// append data to the end of the file
// @TODO, if there are rows marked for deletion, insert there instead
void insert(std::shared_ptr<node> insertRoot) {

    std::string tableName = insertRoot->components[0]->value;
    std::ofstream file(TABLE_DIRECTORY + tableName + FILE_EXTENSION, std::ios_base::app);

    // write the 'delete' byte as 0
    file << static_cast<unsigned char>(0b0);

    auto columnValueListRoot = insertRoot->components[1];

    TableInfo t(TABLE_DIRECTORY + tableName + FILE_EXTENSION);
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
            // std::cout << "Unmentioned: " << c.name;
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

// drop a table
void executeDrop(std::shared_ptr<node> dropRoot)  {
    std::filesystem::remove("../tables/" + dropRoot->components[0]->value + ".ftbl");
}

// given a TableInfo, write a header for a table that does not exist yet
void writeHeader(const TableInfo& table) {
    std::ofstream header(TABLE_DIRECTORY + table.name + FILE_EXTENSION);

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

// called in define()
void defineSelection(std::shared_ptr<node> definitionRoot) {
    std::string definedTableName = definitionRoot->components[1]->value;
    std::ofstream definedFile(TABLE_DIRECTORY + definedTableName + FILE_EXTENSION);

    auto selectionRoot = definitionRoot->components[2];
    auto selectedColumnListRoot = selectionRoot->components[2];
    std::string selectedTableName = selectionRoot->components[1]->value;
    TableInfo selectedInfo(TABLE_DIRECTORY + selectedTableName + FILE_EXTENSION);

    // figure out which columns were selected
    std::vector<ColumnInfo> definedColumns;
    // *
    if (selectedColumnListRoot->components[0]->type == asterisk)
        definedColumns = selectedInfo.columns;
    // column names
    else
        for (auto& columnNode : selectedColumnListRoot->components)
            definedColumns.push_back(*selectedInfo[columnNode->value]);

    // write header
    TableInfo definedInfo(definedTableName, definedColumns);
    writeHeader(definedInfo);

    Table selectedTable(selectedInfo);
    Table definedTable(definedInfo);

    auto whereClauseRoot = selectionRoot->components[3];
    
    // no where clause
    if (whereClauseRoot->type == nullnode) {
        while (selectedTable.nextRow()) {
            char deleteByte = '\0';
            definedTable.appendBytes(&deleteByte, 1);
            for (const ColumnInfo& column : definedColumns) {
                char* selectedBytes = selectedTable.getBytes(column.name);
                definedTable.appendBytes(selectedBytes, column.bytesNeeded);
            }
        }
    }
    // where clause
    else if (whereClauseRoot->type == where_clause) {

        std::shared_ptr<EvaluationNode> evaluationRoot = convert(whereClauseRoot->components[0], selectedTable, selectedInfo);
        while (selectedTable.nextRow()) {
            if (!evaluationRoot->evaluate())
                continue;

            char deleteByte = '\0';
            definedTable.appendBytes(&deleteByte, 1);

            for (const ColumnInfo& column : definedColumns) {
                char* selectedBytes = selectedTable.getBytes(column.name);
                definedTable.appendBytes(selectedBytes, column.bytesNeeded);
            }
        }
    }
}

// called in define()
void defineBagOp(std::shared_ptr<node> definitionRoot) {
    std::string definedTableName = definitionRoot->components[1]->value;
    std::ofstream definedFile(TABLE_DIRECTORY + definedTableName + FILE_EXTENSION);

    auto bagOpRoot = definitionRoot->components[2];
    element_type opType = bagOpRoot->components[0]->type;
    TableInfo table1Info(TABLE_DIRECTORY + bagOpRoot->components[1]->value + FILE_EXTENSION);
    TableInfo table2Info(TABLE_DIRECTORY + bagOpRoot->components[2]->value + FILE_EXTENSION);

    std::vector<ColumnInfo> definedColumns = table1Info.columns;
    // for each chars column, make sure that the larger one is put into workingColumns  
    for (auto& workingColumn : definedColumns) {
        if (workingColumn.type == chars_literal) {
            auto c2 = find(workingColumn.name, table2Info.columns);
            // set longer chars length
            workingColumn.charsLength  = ( c2->charsLength > workingColumn.charsLength ? c2->charsLength : workingColumn.charsLength );
            // set longer bytes needed
            workingColumn.bytesNeeded  = ( c2->bytesNeeded > workingColumn.bytesNeeded ? c2->bytesNeeded : workingColumn.bytesNeeded );
        }
    }

    // write table header
    TableInfo definedInfo(definedTableName, definedColumns);
    writeHeader(definedInfo);

    Table table1(table1Info);
    Table table2(table2Info);
    Table definedTable(definedInfo);

    // bag union
    if (opType == kw_union) {
        // output first table using larger output width
        while (table1.nextRow()) {

            // set row undeleted
            char deleteByte = '\0';
            definedTable.appendBytes(&deleteByte, 1);

            for (const ColumnInfo& column : definedColumns) {

                char* columnBytes = table1.getBytes(column.name);
                int table1BytesNeeded = table1.t[column.name]->bytesNeeded;
                definedTable.appendBytes(columnBytes, table1BytesNeeded);

                // pad shorter chars columns
                if (table1.t[column.name]->type == chars_literal && table1BytesNeeded < column.bytesNeeded) {
                    int paddingLength = column.bytesNeeded - table1BytesNeeded;
                    for (int i = 0; i < paddingLength; ++i) {
                        char nullByte = '\0';
                        definedTable.appendBytes(&nullByte, 1);
                    }
                }
            }
        }
        // output second table using larger output width
        while (table2.nextRow()) {

            // set row undeleted
            char deleteByte = '\0';
            definedTable.appendBytes(&deleteByte, 1);

            for (const ColumnInfo& column : definedColumns) {

                char* columnBytes = table2.getBytes(column.name);
                int table2BytesNeeded = table2.t[column.name]->bytesNeeded;
                definedTable.appendBytes(columnBytes, table2BytesNeeded);

                // pad shorter chars columns
                if (table2.t[column.name]->type == chars_literal && table2BytesNeeded < column.bytesNeeded) {
                    int paddingLength = column.bytesNeeded - table2BytesNeeded;
                    for (int i = 0; i < paddingLength; ++i) {
                        char nullByte = '\0';
                        definedTable.appendBytes(&nullByte, 1);
                    }
                }
            }
        }
    }

    // bag intersect
    else if (opType == kw_intersect)  {
        while (table1.nextRow()) {
            // compare each row of table1 to every row of table2 until a match is found
            while (table2.nextRow()) {
                // match not found, skip
                if (!table1.compareRow(table2))
                    continue;

                // set row undeleted
                char deleteByte = '\0';
                definedTable.appendBytes(&deleteByte, 1);

                // match found. output & reset table2
                for (const ColumnInfo& column : definedColumns) {
                    char* columnBytes;
                    // for chars, just pick the bytes from the longer column. we know they're the same
                    if (column.type == chars_literal)
                        columnBytes = ( table1.t[column.name]->bytesNeeded > table2.t[column.name]->bytesNeeded ? table1.getBytes(column.name) : table2.getBytes(column.name) );
                    else
                        columnBytes = table1.getBytes(column.name);

                    definedTable.appendBytes(columnBytes, column.bytesNeeded);
                }

                table2.reset();
                break;
            }
            // match never found, reset table2
            table2.reset();
        }
    }
}

// define a table
// contains the logic for defining from column, type list
void define(std::shared_ptr<node> definitionRoot) {

    switch (definitionRoot->components[2]->type) {
        case col_type_list: {
            TableInfo newTable = nodeToTableInfo(definitionRoot);
            writeHeader(newTable);
        }
        break;
        
        case selection:
            defineSelection(definitionRoot);
        break;

        case bag_op:
            defineBagOp(definitionRoot);
        break;

        default:
            std::cout << "Defined a table other than from a column, type list or selection. There is likely an issue in the parser. Ignoring.\n";
            exit(1);
    }
}

void execute(std::shared_ptr<node> scriptRoot) {
    for (auto& statementRoot: scriptRoot->components) {
        switch (statementRoot->type) {

            case join:
                executeJoin(statementRoot);
                break;

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