// Table.hpp

#ifndef TABLE
#define TABLE

#include "TableInfo.hpp"
#include <fstream>
#include <iomanip>

#define UNDERLINE "\033[4m"
#define CLOSEUNDERLINE "\033[0m"

std::string TABLE_DIRECTORY = "../tables/";
std::string FILE_EXTENSION = ".ftbl";

struct Table {
    
    TableInfo t;
    std::fstream file;
    char* currentRow;
    char* peekRow;
    unsigned int rowSize;
    unsigned int dataStartPosition;

    // constructor
    Table(TableInfo& t) : t(t) {

        file = std::fstream(TABLE_DIRECTORY + t.name + FILE_EXTENSION);

        // seek to first row
        file.seekg(64);
        char numColumnBuffer[4];
        file.read(numColumnBuffer, 4);
        int numColumns = *(int*)(numColumnBuffer);
        dataStartPosition = 68 + 68 * numColumns;
        file.seekg(dataStartPosition, std::ios_base::beg);

        // get the row size and allocate space
        rowSize = 1;
        for (auto& c : t.columns) {
            rowSize += c.bytesNeeded;
        }
        currentRow = new char[rowSize];
        peekRow = new char[rowSize];
    }

    ~Table() {
        delete [] currentRow;
        delete [] peekRow;
    }

    // check if an row is null (type agnostic)
    bool isNull(const std::string& columnName) {
        return *(uint8_t*)(currentRow + t[columnName]->offset);
    }

    // get int value by column name
    int getInt(const std::string& columnName) {
        return *(int*)(currentRow + t[columnName]->offset + 1);
    }

    // get float value by column name
    float getFloat(const std::string& columnName) {
        return *(float*)(currentRow + t[columnName]->offset + 1);      
    }

    // get chars value by column name
    std::string getChars(const std::string& columnName) {
        ColumnInfo* c = t[columnName];
        std::string str(currentRow + c->offset + 1, c->bytesNeeded);
        str.erase(std::find(str.begin(), str.end(), '\0'), str.end()); // Remove all null characters
        return str;  
    }

    // get bool value by column name
    bool getBool(const std::string& columnName) {
        return *(uint8_t*)(currentRow + t[columnName]->offset + 1);
    }

    // get value as a string
    std::string getValueString(const std::string& columnName) {
        ColumnInfo* c = t[columnName];

        if (isNull(columnName))
            return "$null";

        switch (c->type) {

            case int_literal:
                return std::to_string(getInt(columnName));

            case float_literal:
                return std::to_string(getFloat(columnName));

            case chars_literal:
                return getChars(columnName);

            case bool_literal:
                if (getBool(columnName) == 0)
                    return "false";
                else
                    return "true";

            default:
                std::cout << "Bad column type in Table::getValueString().\n";
                exit(1);
        }
    }

    // write an int
    void setInt(const std::string& columnName, int value) {
        std::streampos current = file.tellg();
        char nullByte = '\0';

        file.seekp(file.tellg() - std::streamoff(rowSize) + std::streamoff(t[columnName]->offset), std::ios_base::beg);
        file.write(&nullByte, 1);
        file.write((char*)&value, sizeof(int));

        file.seekg(current);
    }

    // write a float
    void setFloat(const std::string& columnName, float value) {
        std::streampos current = file.tellg();
        char nullByte = '\0';

        file.seekp(file.tellg() - std::streamoff(rowSize) + std::streamoff(t[columnName]->offset), std::ios_base::beg);
        file.write(&nullByte, 1);
        file.write((char*)&value, sizeof(float));

        file.seekg(current);
    }

    // write chars
    void setChars(const std::string& columnName, const std::string& value) {
        std::streampos current = file.tellg();
        ColumnInfo* c = t[columnName];
        char nullByte = '\0';

        file.seekp(file.tellg() - std::streamoff(rowSize) + std::streamoff(c->offset), std::ios_base::beg);
        file.write(&nullByte, 1);
        file.write(value.c_str(), value.length());
        // pad rest with nulls
        for (int i = 0; i < c->bytesNeeded-value.length(); ++i) 
            file.write(&nullByte, 1);

        file.seekg(current);
    }

    // write a bool
    void setBool(const std::string& columnName, bool value) {
        std::streampos current = file.tellg();
        char nullByte = '\0';
        char trueByte = 1;

        file.seekp(file.tellg() - std::streamoff(rowSize) + std::streamoff(t[columnName]->offset), std::ios_base::beg); 
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
       
        char nullByte = 1;

        file.seekp(file.tellg() - std::streamoff(rowSize) + std::streamoff(t[columnName]->offset), std::ios_base::beg); 
        file.write(&nullByte, 1);

        file.seekg(current);
    }

    // print selected columns of current row to output stream
    void printRow(std::ostream& out, const std::vector<std::string>& columnNames) {
        for (const std::string& name : columnNames)
            out << getValueString(name) << ' ';
        out << '\n';
    }

    // return true if the currentRow is the same as the currentRow of another Table
    // THIS FUNCTION IS ONLY USED FOR INTERSECTS, WHEN IT IS KNOWN THAT TWO TABLES HAVE THE SAME COLUMNS
    bool compareRow(Table& otherTable) {
        for (auto& column : t.columns) {
            switch (column.type) {
                case int_literal:
                    if (this->getInt(column.name) != otherTable.getInt(column.name))
                        return false;
                    break;

                case float_literal:
                    if (this->getFloat(column.name) != otherTable.getFloat(column.name))
                        return false;
                    break;

                case chars_literal:
                    if (this->getChars(column.name) != otherTable.getChars(column.name))
                        return false;
                    break;

                case bool_literal:
                    if (this->getBool(column.name) != otherTable.getBool(column.name))
                        return false;
                    break;
                
                default:
                    std::cout << "Error in Table::compareRow(), likely during an intersect. Column \"" << column.name << "\" in table \"" << t.name << "\" is not one of the literal types.\n";
                    exit(1);
            }
        }
        return true;
    }
    
    // mark current row for deletion
    void markForDeletion() {
        std::streampos current = file.tellg();
       
        char deleteByte = 1;

        file.seekp(file.tellg() - std::streamoff(rowSize), std::ios_base::beg); 
        file.write(&deleteByte, 1);

        file.seekg(current);
    }

    // check if current row is marked for deletion
    bool isMarkedForDeletion() {
        return currentRow[0];
    }

    // return to before first item
    void reset() {
        file.seekg(dataStartPosition, std::ios_base::beg);
    }

    // advance to next non-deleted row
    bool nextRow() {
        while (true) {
            file.read(currentRow, rowSize);

            // stop at end of file
            if (file.eof()) {
                file.clear();  
                return false;  
            }

            if (!isMarkedForDeletion())
                return true;
            
            // if it IS marked for deletion, skip
        }
    }
};

#endif