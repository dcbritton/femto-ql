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
    element_type type;
    int charsLength = 0; // 0 for non-chars
    column(std::string columnName, element_type columnType) : name(columnName), type(columnType), charsLength(0) {}; // for non-chars columns
    column(std::string columnName, element_type columnType, int len) : name(columnName), type(columnType), charsLength(len) {}; // for chars columns
};

struct table {
    std::string name;
    std::vector<column> columns;
    table() : name(""), columns({}) {};
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
        element_type colType;
        switch (columnTypePair->components[1]->type) {
            case kw_int:
                colType = int_literal;
                break;
            case kw_float:
                colType = float_literal;
                break;
            case kw_chars:
                colType = chars_literal;
                break;
            case kw_bool:
                colType = bool_literal;
                break;
            default:
                std::cout << "Error in nodeToTable(). Somehow, an unknown column type was provided.\n";
                exit(1);
        }
        
        // if the type is chars, then we need know how many
        int numChars = 0;
        if (colType == chars_literal) numChars = std::stoi(columnTypePair->components[2]->value);

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

element_type byteToColumnType(char signedByte) {
    unsigned char byte = static_cast<unsigned char>(signedByte);
    std::unordered_map<unsigned char, element_type> typeMap;
    typeMap[0b00000000] = int_literal;
    typeMap[0b00000001] = float_literal;
    typeMap[0b00000010] = bool_literal;
    typeMap[0b00000011] = chars_literal;

    if (typeMap.find(byte) == typeMap.end()) {
        std::cout << "Error while reading a table. Could not recognize column type: \"" << byte << "\"\n";
        exit(1);
    }

    return typeMap[byte];
}

unsigned char columnTypeToByte(element_type columnType) {
    unsigned char byte = '\0';
    std::unordered_map<element_type, unsigned char> typeMap;
    typeMap[int_literal] = 0b00000000;
    typeMap[float_literal] = 0b00000001;
    typeMap[bool_literal] = 0b00000010;
    typeMap[chars_literal] = 0b00000011;

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
            std::cout << c.name << ' ' << tokenTypeToString(c.type) << (c.type == chars_literal ? std::to_string(c.charsLength) : "") << ", ";
        std::cout << '\n';
    }
}

// get table data from a file header, given a path
table tableDataFromFileHeader(const std::string& filePath) {
    // access the file
    std::ifstream tableFile(filePath);

    // read first 64 bytes into table name
    char tableNameBuffer[65];
    tableFile.read(tableNameBuffer, 64);
    tableNameBuffer[64] = '\0';

    // read next 4 bytes into numColumns 
    char numColumnBuffer[4];
    tableFile.read(numColumnBuffer, 4);
    int numColumns = *(int*)(numColumnBuffer);

    // for each column, get the name and type
    std::vector<column> columns;
    int currentPos = 68;
    for (int i = 0; i < numColumns; ++i) {
        // read 64 bytes for name
        tableFile.seekg(currentPos);
        char columnNameBuffer[65];
        tableFile.read(columnNameBuffer, 64);
        columnNameBuffer[64] = '\0';

        // next byte for type
        char columnTypeBuffer[1];
        tableFile.read(columnTypeBuffer, 1);
        element_type columnType = byteToColumnType(columnTypeBuffer[0]);

        unsigned int numChars = 0;
        if (columnType == chars_literal) {
            char numCharsBuffer[3];
            tableFile.read(numCharsBuffer, 3);
            // read only the final byte to get number of chars, this means there are 2 bytes of NUL padding 
            numChars = static_cast<uint8_t>(numCharsBuffer[2]);
        }
        
        columns.push_back(column(std::string(columnNameBuffer), columnType, numChars));
        currentPos += 68;
    }

    return table(tableNameBuffer, columns);
}

// a symbol table is constructed and used during validation to ensure that all referenced tables and columns indeed exist
std::vector<table> buildTableList(const std::string& tableDirectory) {
    std::vector<table> tables;
    for (const auto& file : std::filesystem::directory_iterator(tableDirectory)) {
        tables.push_back(tableDataFromFileHeader(file.path().generic_string()));
    }
    return tables;
}

#endif