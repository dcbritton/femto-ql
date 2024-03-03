// table.hpp

#ifndef TABLE
#define TABLE

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

struct column {
    std::string name;
    std::string type;
    int charsLength = 0; // 0 for non-chars
    column(std::string columnName, std::string columnType) : name(columnName), type(columnType), charsLength(0) {}; // for non-chars columns
    column(std::string columnName, std::string columnType, int len) : name(columnName), type(columnType), charsLength(len) {}; // for chars columns
};

struct table {
    std::string name;
    std::vector<column> columns;
    table(std::string tableName, std::vector<column> tableColumns) : name(tableName), columns(tableColumns) {};
};

// free function to find a table by name in a vector of tables
// iterator may be used to remove a table?
std::vector<table>::const_iterator find(const std::vector<table>& tables, const std::string& tableName) {
    auto it = tables.begin();
    while (it != tables.end()) {
        if (it->name == tableName)
            break;
        ++it;
    }

    return it;
}

std::string byteToColumnType(char signedByte) {
    unsigned char byte = static_cast<unsigned char>(signedByte);
    std::unordered_map<unsigned char, std::string> typeMap;
    typeMap[0b00000000] = "int";
    typeMap[0b00000001] = "float";
    typeMap[0b00000010] = "bool";
    typeMap[0b00000011] = "chars";

    if (typeMap.find(byte) == typeMap.end()) {
        std::cout << "Error while reading a table. Could not recognize column type: \"" << byte << "\"\n";
        exit(1);
    }

    return typeMap[byte];
}

unsigned char columnTypeToByte(const std::string& columnType) {
    unsigned char byte = '\0';
    std::unordered_map<std::string, unsigned char> typeMap;
    typeMap["int"] = 0b00000000;
    typeMap["float"] = 0b00000001;
    typeMap["bool"] = 0b00000010;
    typeMap["chars"] = 0b00000011;

    if (typeMap.find(columnType) == typeMap.end()) {
        std::cout << "Error while reading a table. Unknown datatype: \"" << columnType << "\"\n";
        exit(1);
    }

    return typeMap[columnType];
}

#endif