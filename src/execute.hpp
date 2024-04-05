// execute.hpp

#ifndef EXECUTE
#define EXECUTE

#include <fstream>
#include <iostream>
#include <filesystem>
#include <algorithm>
#include "table.hpp"
#include "node.hpp"

void define(const table& table) {

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

#endif
