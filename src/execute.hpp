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

void insert(std::shared_ptr<node> insertRoot) {
    std::string tableName = insertRoot->components[0]->value;
    auto columnValueListRoot = insertRoot->components[1];

    // auto t = find();
}

void executeDrop(std::shared_ptr<node> dropRoot)  {
    std::filesystem::remove("/tables/" + dropRoot->components[0]->value + ".ftbl");
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