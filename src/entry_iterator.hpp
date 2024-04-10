// entry.hpp

#ifndef ENTRY
#define ENTRY

#include "table.hpp"
#include <vector>
#include <fstream>
#include <unordered_map>
#include <tuple>

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
            std::cout << "Error during executor. Somehow, a column is not one of the literal types.\n";
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

struct EntryIterator {
    
    std::ifstream& file;
    char* currentEntry;
    int entrySize;
    const std::vector<column>& columns;
    std::unordered_map<std::string, ColumnInfo> nameToInfo;

    // constructor
    EntryIterator(std::ifstream& file, const std::vector<column>& columns) : file(file), columns(columns) {
        
        // seek to first entry
        file.seekg(64);
        char numColumnBuffer[4];
        file.read(numColumnBuffer, 4);
        int numColumns = *(int*)(numColumnBuffer);
        file.seekg(68 + 68 * numColumns, std::ios_base::beg);

        int offset = 1;
        entrySize = 1;
        for (auto& c : columns) {
            int bytes = columnBytesNeeded(c);
            // get entry size
            entrySize += bytes;
            // construct name to info map
            nameToInfo[c.name] = ColumnInfo(offset, c.type, bytes);
            offset += bytes;
        }
        currentEntry = new char[entrySize];
    }

    ~EntryIterator() {
        delete [] currentEntry;
    }

    // // subscript, returns value
    // template <class T>
    // T operator[](std::string& columnName) {

    // }

    // get value as a string
    std::string getValueString(std::string& columnName) {
        ColumnInfo info = nameToInfo[columnName];
        switch (info.type) {

            case int_literal:
                return std::to_string(*(int*)(currentEntry + info.offset + 1));

            case float_literal:
                return std::to_string(*(float*)(currentEntry + info.offset + 1));

            case chars_literal:
                return std::string((currentEntry + info.offset + 1), info.bytesToRead);

            case bool_literal:
                if (*(uint8_t*)(currentEntry + info.offset + 1) == 0)
                    return "false";
                else
                    return "true";

            default:
                return "Bad column type in EntryIterator.";
        }
    }

    // advance to next item
    bool next() {
        file.read(currentEntry, entrySize);
        if (file.eof())
            return false;
        return true;
    }
};

#endif 