// execute.hpp

#ifndef EXECUTE
#define EXECUTE

#include <fstream>
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <string>
#include <tuple>
#include "table.hpp"
#include "node.hpp"

std::string DIRECTORY = "../tables/";
std::string FILE_EXTENSION = ".ftbl";

int columnBytesNeeded(const column& c) {
    switch (c.type) {
        case int_literal:
            return 5;

        case float_literal:
            return 5;

        case chars_literal:
            return 1 + c.charsLength;

        case bool_literal:
            return 2;
    
        default:
            std::cout << "Error during executor. Somehow, a column is not one of the literal types.\n";
            exit(1);
    }
}

void select(std::shared_ptr<node> selectionRoot) {
    std::string tableName = selectionRoot->components[1]->value;
    std::ifstream file(DIRECTORY + tableName + FILE_EXTENSION);
    auto tables = buildTableList(DIRECTORY);
    auto t = find(tableName, tables);

    // seek to the beginning of the data
    file.seekg(64);
    char numColumnBuffer[4];
    file.read(numColumnBuffer, 4);
    int numColumns = *(int*)(numColumnBuffer);
    file.seekg(68 + 68 * numColumns, std::ios_base::beg);

    // get offsets & num bytes to read of every column
    std::unordered_map<std::string, int> nameToOffset;
    int offset = 1;
    for (auto& c : t->columns) {
        nameToOffset.insert({c.name, offset});
        offset += columnBytesNeeded(c);
    }

    // @TODO implement * column list
    auto columnList = selectionRoot->components[2];
    if (columnList->components[0]->type == asterisk) {
        
    } 

    // go through ast and see which need to be selected
    std::vector<std::tuple<int, element_type, int>> offsetsAndColumnData;
    for (auto& mentionedColumn : columnList->components) {
        auto c = find(mentionedColumn->value, t->columns);
        offsetsAndColumnData.push_back({nameToOffset[mentionedColumn->value], c->type, columnBytesNeeded(*c)});
    }

    // get entry size and allocate space needed
    int entrySize = 1;
    for (const column& c : t->columns)
        entrySize += columnBytesNeeded(c);
    char* entry = new char[entrySize];

    // read and output entries
    while (!file.eof()) {
        
        file.read(entry, entrySize);
        if (file.eof())
            break;

        // using the offsets and column data, output the selected parts of the entry in order
        for (auto& selected : offsetsAndColumnData) {
            // switch on element type
            // @TODO handle null byte!
            switch (std::get<1>(selected)) {

                case int_literal:
                    std::cout << *(int*)(entry + std::get<0>(selected) + 1) << ' ';
                    break;

                case float_literal:
                    std::cout << *(float*)(entry + std::get<0>(selected) + 1) << ' ';
                    break;

                case chars_literal:
                    std::cout.write((entry + std::get<0>(selected) + 1), std::get<2>(selected)) << ' ';
                    break;

                case bool_literal:
                    if (*(uint8_t*)(entry + std::get<0>(selected) + 1) == 0)
                        std::cout << "false" << ' ';
                    else
                        std::cout << "true" << ' ';
                    break;

                default:
                    break;
            }
        }
        std::cout << '\n';
    }

    delete [] entry;

    // @TODO WHERE CLAUSE
}

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

    auto tables = buildTableList(DIRECTORY);
    auto t = find(tableName, tables);
    for (auto& c : t->columns) {
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