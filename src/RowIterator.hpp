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
    std::fstream file;
    char* currentRow;
    unsigned int rowSize;
    unsigned int dataStartPosition;
    std::unordered_map<std::string, ColumnInfo> nameToInfo;

    // constructor
    RowIterator(table& t) : t(t) {

        file = std::fstream(DIRECTORY + t.name + FILE_EXTENSION);

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
    bool isNull(const std::string& columnName) {
        return *(uint8_t*)(currentRow + nameToInfo[columnName].offset);
    }

    // get int value by column name
    int getInt(const std::string& columnName) {
        return *(int*)(currentRow + nameToInfo[columnName].offset + 1);
    }

    // get float value by column name
    float getFloat(const std::string& columnName) {
        return *(float*)(currentRow + nameToInfo[columnName].offset + 1);      
    }

    // get chars value by column name
    std::string getChars(const std::string& columnName) {
        ColumnInfo info = nameToInfo[columnName];
        std::string str(currentRow + info.offset + 1, info.bytesToRead);
        str.erase(std::find(str.begin(), str.end(), '\0'), str.end()); // Remove all null characters
        return str;  
    }

    // get bool value by column name
    bool getBool(const std::string& columnName) {
        return *(uint8_t*)(currentRow + nameToInfo[columnName].offset + 1);
    }

    // get value as a string
    std::string getValueString(const std::string& columnName) {
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

    // write an int
    void setInt(const std::string& columnName, int value) {
        std::streampos current = file.tellg();
       
        ColumnInfo info = nameToInfo[columnName];
        char nullByte = '\0';

        file.seekp(file.tellg() - std::streamoff(rowSize) + std::streamoff(info.offset), std::ios_base::beg);
        file.write(&nullByte, 1);
        file.write((char*)&value, sizeof(int));

        file.seekg(current);
    }

    // write a float
    void setFloat(const std::string& columnName, float value) {
        std::streampos current = file.tellg();
       
        ColumnInfo info = nameToInfo[columnName];
        char nullByte = '\0';

        file.seekp(file.tellg() - std::streamoff(rowSize) + std::streamoff(info.offset), std::ios_base::beg);
        file.write(&nullByte, 1);
        file.write((char*)&value, sizeof(float));

        file.seekg(current);
    }

    // write chars
    void setChars(const std::string& columnName, const std::string& value) {
        std::streampos current = file.tellg();
       
        ColumnInfo info = nameToInfo[columnName];
        char nullByte = '\0';

        file.seekp(file.tellg() - std::streamoff(rowSize) + std::streamoff(info.offset), std::ios_base::beg);
        file.write(&nullByte, 1);
        file.write(value.c_str(), value.length());
        // pad rest with nulls
        for (int i = 0; i < info.bytesToRead-value.length(); ++i) 
            file.write(&nullByte, 1);

        file.seekg(current);
    }

    // write a bool
    void setBool(const std::string& columnName, bool value) {
        std::streampos current = file.tellg();
       
        ColumnInfo info = nameToInfo[columnName];
        char nullByte = '\0';
        char trueByte = 1;

        file.seekp(file.tellg() - std::streamoff(rowSize) + std::streamoff(info.offset), std::ios_base::beg); 
        file.write(&nullByte, 1);
        if(value)
            file.write((char*)&trueByte, 1);
        // use nullByte for false
        else
            file.write((char*)&nullByte, 1);

        file.seekg(current);
    }

    // set a cell's null byte
    void setNull(const std::string& columnName) {
        std::streampos current = file.tellg();
       
        ColumnInfo info = nameToInfo[columnName];
        char nullByte = 1;

        file.seekp(file.tellg() - std::streamoff(rowSize) + std::streamoff(info.offset), std::ios_base::beg); 
        file.write(&nullByte, 1);

        file.seekg(current);
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