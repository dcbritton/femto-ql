// tokenize.hpp

#ifndef TOKENIZE
#define TOKENIZE

#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cctype>

const int MAX_IDENTIFIER_LENGTH = 64;

// remove everything in [#,\n)
// keep \n for accurate line number in parse errors
void remove_comments(std::string& script) {
    std::string::iterator it = script.begin();
    std::string::iterator comment_end;
    
    // std::cout << "Extracted comments:\n-------------------\n";
    while (it != script.end()) {
        if (it == script.end()) break; // Ensure we don't go past the end
        it = std::find(it, script.end(), '#');
        comment_end = std::find(it, script.end(), '\n');

        // outputs list of comments each on a new line
        // std::cout << std::string(it, comment_end) << "\n";

        script.erase(it, comment_end);
    }
    // std::cout << '\n';
}

// outputs the whole script with comments removed, newlines as \n, tabs as \t, and four spaces as \4
void print_escaped_whitespace(const std::string& script) {
    std::cout << "Script with removed comments:\n-----------------------------\n";
    for (auto sit = script.begin(); sit != script.end(); ++sit) {
        // newline to "\\n"
        // if (*sit == '\n' ) {
        //     std::cout << "\\n";
        //     continue;
        // }
        // // 
        // if (*sit == '\t' ) {
        //     std::cout << "\\t";
        //     continue;
        // }
        // else if (*sit == ' ') {
        //     auto tab_end = sit;
        //     while (*tab_end == ' ' && tab_end - sit < 4)
        //         ++tab_end;
        //     auto spaces = std::string(sit, tab_end);
        //     if (spaces.length() == 4)
        //         std::cout << "\\4";
        //     if (spaces.length() < 4) 
        //         std::cout << spaces;
        //     sit = tab_end - 1;
        //     continue;
        // }
        std::cout << *sit;
    }
    std::cout << "\n\n";
}

// print token stream
void print_token_stream(const std::vector<token>& token_stream) {

    std::cout << "Token stream:\n-------------\n";
    std::cout << "Values: ";
    for (auto x : token_stream) 
        std::cout << x.value << ' ';
    std::cout << '\n';
    std::cout << "Types: ";
    for (auto x : token_stream) 
        std::cout << x.type << ' ';
    std::cout << "\n\n";
}

// tokenizer design based based on DFAs in chapter 2, from Engineering a Compiler 2nd Edition by Cooper & Torczon,
std::vector<token> tokenize(const std::string& statement) {
    unsigned int line_number = 1;

    std::unordered_map<std::string, element_type> keyword_map;
    keyword_map["select"] = kw_select;
    keyword_map["from"] = kw_from;
    keyword_map["where"] = kw_where;
    keyword_map["distinct"] = kw_distinct;
    keyword_map["order"] = kw_order;
    keyword_map["asc"] = kw_asc;
    keyword_map["desc"] = kw_desc;
    keyword_map["true"] = kw_true;
    keyword_map["false"] = kw_false;
    keyword_map["null"] = kw_null;
    keyword_map["in"] = kw_in;
    keyword_map["any"] = kw_any;
    keyword_map["all"] = kw_all;
    keyword_map["define"] = kw_define;
    keyword_map["as"] = kw_as;
    keyword_map["join"] = kw_join;
    keyword_map["on"] = kw_on;
    keyword_map["union"] = kw_union;
    keyword_map["intersect"] = kw_intersect;
    keyword_map["delete"] = kw_delete;
    keyword_map["insert"] = kw_insert;
    keyword_map["into"] = kw_into;
    keyword_map["update"] = kw_update;
    keyword_map["int"] = kw_int;
    keyword_map["float"] = kw_float;
    keyword_map["bool"] = kw_bool;
    keyword_map["chars"] = kw_chars;
    keyword_map["drop"] = kw_drop;
    keyword_map["with"] = kw_with;
    keyword_map["temporary"] = kw_temporary;

    // begin traversing input string
    std::vector<token> tokens = {};
    std::string::const_iterator it = statement.begin();
    while (it != statement.end()) {

        // ignore all whitespace
        if (isspace(*it)) {
            if (*it == '\n')
                ++line_number;
            ++it;
        }

        // potential identifier or keyword, begins with alpha
        else if (isalpha(*it)) {

            // iterate until end of word
            std::string::const_iterator word_end = it;
            while (isalnum(*word_end)) {        
                word_end++;
            }
            std::string word(it, word_end);

            // check length
            if (word.length() > MAX_IDENTIFIER_LENGTH) {
                std::cout << "Tokenization error on line " << line_number << ". Table/column name " << word << " is greater than 64 characters long.\n";
                exit(1);
            }

            // if it contains a '.', we know the following must be a column name
            if (*word_end == '.') {
                word_end++;
                std::string::const_iterator column_name_begin = word_end;

                if (isalpha(*word_end)) {
                    word_end++;
                    while(isalnum(*word_end)) {
                        word_end++;
                    }
                }
                else {
                    std::cout << "Tokenization error. Expecting alpha after '.'. Had: \'" << *word_end << "\' instead.\n";
                    exit(1);
                }

                std::string column_name(column_name_begin, word_end);
                // check length
                if (column_name.length() > MAX_IDENTIFIER_LENGTH) {
                    std::cout << "Tokenization error on line " << line_number << ". Column name " << column_name << " is greater than 64 characters long.\n";
                    exit(1);
                }
            }

            word = std::string(it, word_end);

            // identifier found if not in keyword map
            std::unordered_map<std::string, element_type>::const_iterator found_keyword = keyword_map.find(word);
            if (found_keyword == keyword_map.end()) {
                tokens.push_back(token(identifier, word, line_number));
            }
            // otherwise keyword
            else {
                tokens.push_back(token(found_keyword->second, word, line_number));
            }
            it = word_end;
        }
        
        // float or integer literal
        else if (isdigit(*it) || *it == '-') {
            if (*it == '-' && !isdigit(*(it+1))) {
                std::cout << "Tokenization error on line " << line_number << ". '-' must be followed by a digit.\n";
                exit(1);
            }
            std::string::const_iterator number_end = it + 1;
            while (isdigit(*number_end))
                number_end++;
            
            // float
            if (*number_end == '.') {
                number_end++; // consume .
                if (!isdigit(*number_end)) {
                    std::cout << "Tokenization error. Expecting digit after '.'. Had \'" << *number_end << "\' instead.\n";
                    exit(1);
                }
                while (isdigit(*number_end))
                    number_end++;
            
                std::string number(it, number_end);
                tokens.push_back(token(float_literal, number, line_number));
                it = number_end;
                continue;
            }

            // integer, no internal .
            std::string number(it, number_end);
            try {
                std::stoi(number);
            }
            catch (const std::out_of_range& e) {
                std::cout << "Tokenization error on line " << line_number << ". The integer " << number << " out of 32 bit int range.\n";
                exit(1);
            }

            tokens.push_back(token(int_literal, number, line_number));
            it = number_end;
        }

        else if (*it == '\"') {
            std::string::const_iterator chars_end = it;
            chars_end++; // consume "
            while (*chars_end != '\"' && chars_end != statement.end()) {
                chars_end++;
            }

            if (chars_end == statement.end()) {
                std::cout << "Tokenizer error. Unpaired \" on line " << line_number << ".\n";
                exit(1);
            }
            chars_end++; // consume ending "
            
            // keep the value BETWEEN the "..." 
            std::string chars(it+1, chars_end-1);
            tokens.push_back(token(chars_literal, chars, line_number));
            it = chars_end;
        }

        // punctuation and operators
        else if (*it == '(') {
            tokens.push_back(token(open_parenthesis, "(", line_number));
            it++;
        }

        else if (*it == ')') {
            tokens.push_back(token(close_parenthesis, ")", line_number));
            it++;
        }

        else if (*it == '*') {
            tokens.push_back(token(asterisk, "*", line_number));
            it++;
        }

        else if (*it == ',') {
            tokens.push_back(token(comma, ",", line_number));
            it++;
        }

        else if (*it == ':') {
            tokens.push_back(token(colon, ":", line_number));
            it++;
        }

        else if (*it == '=') {
            it++;
            if (*it != '=') {
                std::cout << "Error while tokenizing. Expected ==\n";
                exit(1);
            }
            it++;
            tokens.push_back(token(op_equals, "==", line_number));
        }

        else if (*it == '!') {
            it++;
            if (*it == '=') {
                it++;
                tokens.push_back(token(op_not_equals, "!=", line_number));   
            }
            else 
                tokens.push_back(token(op_not, "!", line_number));
        }

        else if (*it == '<') {
            it++;
            if (*it == '=') {
                it++;
                tokens.push_back(token(op_less_than_equals, "<=", line_number));
            }
            else
                tokens.push_back(token(op_less_than, "<", line_number));
        }

        else if (*it == '>') {
            it++;
            if (*it == '=') {
                it++;
                tokens.push_back(token(op_greater_than_equals, ">=", line_number));
            }
            else 
                tokens.push_back(token(op_greater_than, ">", line_number));
        }
 
        else if (*it == '&') {
            it++;
            if (*it != '&') {
                std::cout << "Error while tokenizing. Expected &&\n";
                exit(1);
            }
            it++;
            tokens.push_back(token(op_and, "&&", line_number));
        }

        else if (*it == '|') {
            it++;
            if (*it != '|') {
                std::cout << "Error while tokenizing. Expected ||\n";
                exit(1);
            }
            it++;
            tokens.push_back(token(op_or, "||", line_number));
        }

        else {
            std::cout << "Unrecognized token at: " << *it << " on line " << line_number << '\n';
            exit(1);
        }
        
    }

    return tokens;
}

#endif