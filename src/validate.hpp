// validate.hpp

#ifndef VALIDATE
#define VALIDATE

#include <fstream>
#include <iostream>
#include <filesystem>
#include "table.hpp"

void printSymbolTable(std::vector<table> symbols) {
    std::cout << "Current Tables:\n---------------\n";
    for (table t : symbols) {
        std::cout << "Table: \"" << t.name << "\"\n";
        std::cout << "        ";
        for (column c : t.columns)
            std::cout << c.name << ' ' << c.type << (c.type == "chars" ? std::to_string(c.charsLength) : "") << ", ";
        std::cout << '\n';
    }
}

// a symbol table is constructed and used during validation to ensure that all referenced tables and columns indeed exist
std::vector<table> buildSymbolTable(const std::string& tableDirectory) {
    std::vector<table> tables;
    // for each table
    for (const auto& file : std::filesystem::directory_iterator(tableDirectory)) {

        // access the file
        std::ifstream tableFile(file.path());

        // read first 64 bytes into table name
        char tableNameBuffer[64];
        tableFile.read(tableNameBuffer, 64);

        // read next 4 bytes into numColumns 
        char numColumnBuffer[4];
        tableFile.read(numColumnBuffer, 4);
        int numColumns = static_cast<int>(static_cast<unsigned char>(numColumnBuffer[0]) << 24 |
                                          static_cast<unsigned char>(numColumnBuffer[1]) << 16 | 
                                          static_cast<unsigned char>(numColumnBuffer[2]) << 8 | 
                                          static_cast<unsigned char>(numColumnBuffer[3]));

        // for each column, get the name and type
        std::vector<column> columns;
        int currentPos = 68;
        for (int i = 0; i < numColumns; ++i) {
            // read 64 bytes for name
            tableFile.seekg(currentPos);
            char columnNameBuffer[64];
            tableFile.read(columnNameBuffer, 64);

            // next byte for type
            char columnTypeBuffer[1];
            tableFile.read(columnTypeBuffer, 1);
            std::string columnType = byteToColumnType(columnTypeBuffer[0]);

            int numChars = 0;
            if (columnType == "chars") {
                char numCharsBuffer[3];
                tableFile.read(numCharsBuffer, 3);
                numChars = static_cast<int>(0 |
                                            static_cast<unsigned char>(numCharsBuffer[0]) << 16 |
                                            static_cast<unsigned char>(numCharsBuffer[1]) << 8 | 
                                            static_cast<unsigned char>(numCharsBuffer[2]));
            }
            
            columns.push_back(column(columnNameBuffer, columnType, numChars));
            currentPos += 68;
        }

        tables.push_back(table(tableNameBuffer, columns));
    }

    return tables;
}

void validate() {
    return;
}

#endif