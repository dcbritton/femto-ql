// create.cpp
// this file is temporary, defining a function to create the file format.

#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <utility>

using std::vector, std::pair, std::string;

struct column {
    string name;
    string type;
    string charsLength; // null for non-chars
    column(string columnName, string columnType) : name(columnName), type(columnType), charsLength("") {}; // for non-chars columns
    column(string columnName, string columnType, string len) : name(columnName), type(columnType), charsLength(len) {}; // for chars columns
};

unsigned char columnTypeToByte(const string& columnType) {
    unsigned char byte = '\0';
    switch (columnType[0]) {
    case 'i':
        byte = static_cast<unsigned char>(0b00000000); // NUL
        break;
    case 'f':
        byte = static_cast<unsigned char>(0b00000001); // SOH
        break;
    case 'b':
        byte = static_cast<unsigned char>(0b00000010); // STX
        break;
    case 'c':
        byte = static_cast<unsigned char>(0b00000011); // EOT
        break;
    default:
        std::cout << "Error in reading table. Unknown datatype: \"" << columnType << "\"\n";
        break;
    }
    return byte;
}

void create(string tableName, vector<column> columns) {

    std::ofstream header;
    header.open("../tables/" + tableName + ".data");

    // bytes 0-63 for tableName
    if (tableName.length() > 64) {
        std::cout << "Creation error. Table name \"" << "\" is longer than 64 bytes.\n";
        exit(1);
    }
    header << tableName;
    // fill the rest with NUL if not exactly 64 bytes.
    for (int i = 0; i < 64 - tableName.length(); ++i)
        header << '\0';

    // next byte reserved for # of columns
    unsigned int numColumns = columns.size();
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
    for (const auto& col : columns) {
        // bytes 0-63 for tableName
        if (col.name.length() > 64) {
            std::cout << "Creation error. Column name \"" << col.name << "\" in table \"" << tableName << "\" is longer than 64 bytes.\n";
            exit(1);
        }
        header << col.name;
        // fill the rest with NUL if not exactly 64 bytes.
        for (int i = 0; i < 64 - col.name.length(); ++i)
            header << '\0';

        // first byte for type
        header << columnTypeToByte(col.type);

        // type is chars
        if (col.type == "chars") {
            // last 3 bytes indicate # of chars
            unsigned int numChars = stoi(col.charsLength);
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

int main(int argc, char const *argv[]) {
    create("This should be a 35 byte table name",
            {{"WordWordWordWordWordWordWordWordWordWordWordWordWordWordWordWord", "int"},
            {"WordWordWordWordWordWordWordWordWordWordWordWordWordWordWordWord", "float"},
            {"WordWordWordWordWordWordWordWordWordWordWordWordWordWordWordWord", "bool"},
            {"WordWordWordWordWordWordWordWordWordWordWordWordWordWordWordWord", "chars", "16"}});
    return 0;
}
