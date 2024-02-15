// tokenize.hpp

#ifndef TOKENIZE
#define TOKENIZE

#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cctype>

// tokenizer design based based on DFAs in chapter 2, from Engineering a Compiler 2nd Edition by Cooper & Torczon,
std::vector<token> tokenize(const std::string& statement) {

    std::unordered_map<std::string, element_type> keyword_map;
    keyword_map["select"] = kw_select;
    keyword_map["from"] = kw_from;
    keyword_map["where"] = kw_where;
    keyword_map["distinct"] = kw_distinct;
    keyword_map["order"] = kw_order;
    keyword_map["asc"] = kw_asc;
    keyword_map["desc"] = kw_desc;

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
            while (isalnum(*word_end)) {        
                word_end++;
            }

            if (*word_end == '.') {
                word_end++;

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
            }
            std::string word(it, word_end);

            // identifier found
            std::unordered_map<std::string, element_type>::const_iterator found_keyword = keyword_map.find(word);
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

        else if (*it == '*') {
            tokens.push_back(token(asterisk, "*"));
            it++;
        }

        else if (*it == ',') {
            tokens.push_back(token(comma, ","));
            it++;
        }

        else if (*it == ';') {
            tokens.push_back(token(semicolon, ";"));
            it++;
        }

        else if (*it == '=') {
            it++;
            if (*it != '=') {
                std::cout << "Error while tokenizing. Expected ==\n";
                exit(1);
            }
            it++;
            tokens.push_back(token(op_equals, "=="));
        }

        else if (*it == '!') {
            it++;
            if (*it == '=') {
                it++;
                tokens.push_back(token(op_not_equals, "!="));   
            }
            else 
                tokens.push_back(token(op_not, "!"));
        }

        else if (*it == '<') {
            it++;
            if (*it == '=') {
                it++;
                tokens.push_back(token(op_less_than_equals, "<="));
            }
            else
                tokens.push_back(token(op_less_than, "<"));
        }

        else if (*it == '>') {
            it++;
            if (*it == '=') {
                it++;
                tokens.push_back(token(op_greater_than_equals, ">="));
            }
            else 
                tokens.push_back(token(op_greater_than, ">"));
        }
 
        else if (*it == '&') {
            it++;
            if (*it != '&') {
                std::cout << "Error while tokenizing. Expected &&\n";
                exit(1);
            }
            it++;
            tokens.push_back(token(op_and, "&&"));
        }

        else if (*it == '|') {
            it++;
            if (*it != '|') {
                std::cout << "Error while tokenizing. Expected ||\n";
                exit(1);
            }
            it++;
            tokens.push_back(token(op_or, "||"));
        }

        else {
            std::cout << "Unrecognized token at: " << *it << '\n';
            exit(1);
        }
        
    }

    return tokens;
}

#endif