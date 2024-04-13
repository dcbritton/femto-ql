// RowIterator.hpp

#ifndef ROWITERATOR
#define ROWITERATOR

#include "table.hpp"
#include <vector>
#include <fstream>
#include <unordered_map>

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
            std::cout << "Error during execution. Somehow, a column is not one of the literal types.\n";
            exit(1);
    }
}

struct ColumnInfo {
    int offset;
    element_type type;
    int bytesToRead;

    ColumnInfo() : offset(0), type(nullnode), bytesToRead(0) {}

    ColumnInfo(int offset, element_type type, int bytesToRead) 
        : offset(offset), type(type), bytesToRead(bytesToRead) {}
};

struct RowIterator {
    
    table t;
    std::ifstream file;
    char* currentRow;
    unsigned int rowSize;
    unsigned int dataStartPosition;
    std::unordered_map<std::string, ColumnInfo> nameToInfo;

    // constructor
    RowIterator(table& t) : t(t) {

        file = std::ifstream(DIRECTORY + t.name + FILE_EXTENSION);

        // seek to first row
        file.seekg(64);
        char numColumnBuffer[4];
        file.read(numColumnBuffer, 4);
        int numColumns = *(int*)(numColumnBuffer);
        dataStartPosition = 68 + 68 * numColumns;
        file.seekg(dataStartPosition, std::ios_base::beg);

        int offset = 1;
        rowSize = 1;
        for (auto& c : t.columns) {
            int bytes = columnBytesNeeded(c);
            // get row size
            rowSize += bytes;
            // construct name to info map
            nameToInfo[c.name] = ColumnInfo(offset, c.type, bytes);
            offset += bytes;
        }
        currentRow = new char[rowSize];
    }

    ~RowIterator() {
        delete [] currentRow;
    }

    // check if an row is null (type agnostic)
    bool isNull(std::string& columnName) {
        return *(uint8_t*)(currentRow + nameToInfo[columnName].offset);
    }

    // get int value by column name
    int getInt(std::string& columnName) {
        return *(int*)(currentRow + nameToInfo[columnName].offset + 1);
    }

    // get float value by column name
    float getFloat(std::string& columnName) {
        return *(float*)(currentRow + nameToInfo[columnName].offset + 1);      
    }

    // get chars value by column name
    std::string getChars(std::string& columnName) {
        ColumnInfo info = nameToInfo[columnName];
        std::string str(currentRow + info.offset + 1, info.bytesToRead);
        str.erase(std::find(str.begin(), str.end(), '\0'), str.end()); // Remove all null characters
        return str;  
    }

    // get bool value by column name
    bool getBool(std::string& columnName) {
        return *(uint8_t*)(currentRow + nameToInfo[columnName].offset + 1);
    }

    // get value as a string
    std::string getValueString(std::string& columnName) {
        ColumnInfo info = nameToInfo[columnName];

        if (isNull(columnName))
            return "**null**";

        switch (info.type) {

            case int_literal:
                return std::to_string(*(int*)(currentRow + info.offset + 1));

            case float_literal:
                return std::to_string(*(float*)(currentRow + info.offset + 1));

            case chars_literal:
                return std::string((currentRow + info.offset + 1), info.bytesToRead);

            case bool_literal:
                if (*(uint8_t*)(currentRow + info.offset + 1) == 0)
                    return "false";
                else
                    return "true";

            default:
                return "Bad column type in EowIterator.";
        }
    }

    // return to before first item
    void reset() {
        file.seekg(dataStartPosition, std::ios_base::beg);
    }

    // advance to next item
    bool next() {
        file.read(currentRow, rowSize);
        if (file.eof()) {
            file.clear();  
            return false;  
        }
        return true;
    }
};

#endif