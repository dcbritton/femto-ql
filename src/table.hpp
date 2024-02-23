// table.hpp

#ifndef TABLE
#define TABLE

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

struct column {
    std::string name;
    std::string type;
    std::string charsLength; // null for non-chars
    column(std::string columnName, std::string columnType) : name(columnName), type(columnType), charsLength("") {}; // for non-chars columns
    column(std::string columnName, std::string columnType, std::string len) : name(columnName), type(columnType), charsLength(len) {}; // for chars columns
};

struct table {
    std::string name;
    std::vector<column> columns;
    table(std::string tableName, std::vector<column> tableColumns) : name(tableName), columns(tableColumns) {};
};

#endif