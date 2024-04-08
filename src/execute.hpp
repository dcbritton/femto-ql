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

void insert(std::shared_ptr<node> insertRoot) {
    std::string tableName = insertRoot->components[0]->value;
    auto columnValueListRoot = insertRoot->components[1];

    // auto t = find();
}

void executeDrop(std::shared_ptr<node> dropRoot)  {
    std::filesystem::remove("/tables/" + dropRoot->components[0]->value + ".ftbl");
}

void define(std::shared_ptr<node> definitionRoot) {
    
    auto table = nodeToTable(definitionRoot);

    std::ofstream header;
    header.open("../tables/" + table.name + ".ftbl");

    // bytes 0-63 for tableName
    if (table.name.length() > 64) {
        std::cout << "Creation error. Table name \"" << "\" is longer than 64 bytes.\n";
        exit(1);
    }
    header << table.name;
    // fill the rest with NUL if not exactly 64 bytes.
    for (int i = 0; i < 64 - table.name.length(); ++i)
        header << '\0';

    // next byte reserved for # of columns
    unsigned int numColumns = table.columns.size();
    // get int as bytes in a buffer
    unsigned char buffer[4];
    for (int i = 3; i >= 0; --i) {
        buffer[i] = static_cast<unsigned char>(numColumns & 0b11111111); // ETX
        numColumns >>= 8;
    }
    // output bytes 64-67
    for (auto x : buffer)
        header << x;

    // 64 bytes for column name followed by 4 bytes for type
    for (const auto& col : table.columns) {
        // bytes 0-63 for tableName
        if (col.name.length() > 64) {
            std::cout << "Creation error. Column name \"" << col.name << "\" in table \"" << table.name << "\" is longer than 64 bytes.\n";
            exit(1);
        }
        header << col.name;
        // fill the rest with NUL if not exactly 64 bytes.
        for (int i = 0; i < 64 - col.name.length(); ++i)
            header << '\0';

        // first byte for type
        header << columnTypeToByte(col.type);

        // type is chars
        if (col.type == chars_literal) {
            // last 3 bytes indicate # of chars
            unsigned int numChars = col.charsLength;
            unsigned char charsLengthBuffer[3];
            for (int i = 2; i >= 0; --i) {
                charsLengthBuffer[i] = static_cast<unsigned char>(numChars & 0b11111111); // ETX
                numChars >>= 8;
            }
            for (auto x : charsLengthBuffer)
                header << x;
        }
        // other types
        else {
            // 3 bytes for padding
            for (int i = 0; i < 3; ++i) 
                header << '\0';
        }
    }

    header.close();
}

void execute(std::shared_ptr<node> scriptRoot) {
    for (auto& statementRoot: scriptRoot->components) {
        switch (statementRoot->type) {

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
