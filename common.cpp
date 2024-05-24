#include "common.h"

void split(const string& str, char delimiter, vector<string>& tokens) {
    size_t start = 0;
    size_t end = 0;

    while ((end = str.find(delimiter, start)) != string::npos) {
        tokens.push_back(str.substr(start, end - start));
        start = end + 1;
    }

    tokens.push_back(str.substr(start));
}