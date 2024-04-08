// execute.hpp

#ifndef EXECUTE
#define EXECUTE

#include <fstream>
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <string>
#include "table.hpp"
#include "node.hpp"

std::string DIRECTORY = "../tables/";
std::string FILE_EXTENSION = ".ftbl";

void writeValue(const std::string& value, element_type type, std::ofstream& file) {
    switch (type) {
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
            break;
        }
        case bool_literal: {
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
        for (auto& columnValuePair : columnValueListRoot->components) {
            // @TODO simplify logic
            int numBytesToWrite = 4;
            // column mentioned
            if (columnValuePair->components[0]->value == c.name) {
                // null insert, write null byte as 1, fill rest as 0
                if (columnValuePair->components[1]->type == kw_null) {
                    //     file << static_cast<unsigned char>(0b1);
                    // if (c.type == chars_literal)
                    //     numBytesToWrite = c.charsLength;
                    // for (int i = 0; i < numBytesToWrite; ++i)
                    //     file << static_cast<unsigned char>(0b0);
                }
                // value to insert
                else {
                    file << static_cast<unsigned char>(0b0);
                    writeValue(columnValuePair->components[1]->value, c.type, file);
                }
            }
            // otherwise, write null byte as 1, fill rest as 0
            else {
                // file << static_cast<unsigned char>(0b1);
                // if (c.type == chars_literal)
                //     numBytesToWrite = c.charsLength;
                // for (int i = 0; i < numBytesToWrite; ++i)
                //     file << static_cast<unsigned char>(0b0);
            }
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

            case definition:
                std::cout << "Executing a definition." << std::endl;
                define(statementRoot);
                break;

            case insertion:
                std::cout << "Executing an insert." << std::endl;
                insert(statementRoot);
                break;
            
            case drop:
                std::cout << "Executing a drop." << std::endl;
                executeDrop(statementRoot);
                break;

            default:
                std::cout << "Unknown statement type to execute.\n";
        }
    }
}

#endif
