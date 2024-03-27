// table.hpp

#ifndef TABLE
#define TABLE

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <optional>
#include "node.hpp"

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

// identifier has '.'
bool hasDot(const std::string& name) {
    return std::find(name.begin(), name.end(), '.') != name.end();
}

// split an identifier name on the '.'
// 2nd string will be "" if no '.' is present
std::pair<std::string, std::string> split(const std::string& name) {
    // find '.'
    int dotPosition = name.find('.');

    // no '.'
    if (std::find(name.begin(), name.end(), '.') == name.end())  {
        return std::make_pair(name, "");
    }
    
    // std::cout << "TEST SPLIT: " << name.substr(0, dotPosition) << " : " << name.substr(dotPosition+1, name.length()-1) << "\n";
    return std::make_pair(name.substr(0, dotPosition), name.substr(dotPosition+1, name.length()-1));
} 

// create a table struct from a node
table nodeToTable(const std::shared_ptr<node>& n) {
    // node must be a definition
    if (n->type != definition) {
        std::cout << "Error converting node to table: Node is not a definition!\n";
        exit(1);        
    }

    std::vector<column> cols;
    for (auto& columnTypePair : n->components[2]->components) {
        // identifier name stored in value
        std::string colName = columnTypePair->components[0]->value;
        // for kw_chars, kw_int, kw_float, kw_bool, there's no value, so infer by type
        std::string colType = tokenTypeToString(columnTypePair->components[1]->type);
        // if the type is chars, then we need know how many
        int numChars = 0;
        if (colType == "chars") numChars = std::stoi(columnTypePair->components[2]->value);

        cols.push_back(column(colName, colType, numChars));
    }

    return (table(n->components[1]->value, cols));
}

// free function to find a table by name in a vector of tables
// iterator may be used to remove a table?
std::vector<column>::const_iterator find(const std::string& columnName, const std::vector<column>& columns) {
    auto it = columns.begin();
    while (it != columns.end()) {
        if (it->name == columnName)
            break;
        ++it;
    }

    return it;
}

// free function to find a table by name in a vector of tables
// iterator may be used to remove a table?
std::vector<table>::const_iterator find(const std::string& tableName, const std::vector<table>& tables) {
    auto it = tables.begin();
    while (it != tables.end()) {
        if (it->name == tableName)
            break;
        ++it;
    }

    return it;
}

// column exists in a vector of columns
bool exists(const std::string& columnName, const std::vector<column>& columns) {
    return std::find_if(columns.begin(), columns.end(), [&columnName](const auto& c){return c.name == columnName;}) != columns.end();
}

// table exists in a vector of tables
bool exists(const std::string& tableName, const std::vector<table>& tables) {
    return find(tableName, tables) != tables.end();
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

void printTableList(std::vector<table> tables) {
    for (table t : tables) {
        std::cout << "\"" << t.name << "\": ";
        for (column c : t.columns)
            std::cout << c.name << ' ' << c.type << (c.type == "chars" ? std::to_string(c.charsLength) : "") << ", ";
        std::cout << '\n';
    }
}

// a symbol table is constructed and used during validation to ensure that all referenced tables and columns indeed exist
std::vector<table> buildTableList(const std::string& tableDirectory) {
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

#endif