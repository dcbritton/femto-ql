#include <iostream>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cctype>
#include "token.hpp"

// tokenizer design based based on DFAs in chapter 2, from Engineering a Compiler 2nd Edition by Cooper & Torczon,
std::vector<token> tokenize(const std::string& statement) {

    std::unordered_map<std::string, token_type> keyword_map;
    keyword_map["select"] = kw_select;
    keyword_map["from"] = kw_from;
    keyword_map["where"] = kw_where;

    // begin traversing input string
    std::vector<token> tokens = {};
    std::string::const_iterator it = statement.begin();
    while (it != statement.end()) {

        // ignore all whitespace
        if (isspace(*it)) {
            it++;
        }

        // potential identifier or keyword, begins with alpha
        else if (isalpha(*it)) {

            // iterate until end of word
            std::string::const_iterator word_end = it;
            while (isalnum(*word_end)) 
                word_end++;
            std::string word(it, word_end);

            // identifier found
            std::unordered_map<std::string, token_type>::const_iterator found_keyword = keyword_map.find(word);
            if (found_keyword == keyword_map.end()) {
                tokens.push_back(token(identifier, word));
            }
            // otherwise keyword
            else {
                tokens.push_back(token(found_keyword->second, word));
            }
            it = word_end;
        }
        
        // potential integer literal
        else if (isdigit(*it)) {
            std::string::const_iterator number_end = it;
            while (isdigit(*number_end))
                number_end++;
            std::string number(it, number_end);
            tokens.push_back(token(int_literal, number));
            it = number_end;
        }

        // punctuation and operators
        else if (*it == '(') {
            tokens.push_back(token(open_parenthesis, "("));
            it++;
        }

        else if (*it == ')') {
            tokens.push_back(token(close_parenthesis, ")"));
            it++;
        }

        else if (*it == ';') {
            tokens.push_back(token(semicolon, ";"));
            it++;
        }

        else if (*it == '=') {
            tokens.push_back(token(operator_equals, "="));
            it++;
        }

        else {
            std::cout << "Unrecognized token";
            exit(1);
        }
        
    }

    return tokens;
}

int main() {
    std::cout << "Enter your query: ";
    std::string statement;
    getline(std::cin, statement);

    std::vector<token> token_stream = tokenize(statement);
    print_token_stream(token_stream);

    std::cout << "Program has ended properly.\n";
    return 0;
}