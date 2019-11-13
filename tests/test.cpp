#include <unistd.h>
#include <cstring>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "../include/parser.h"

int main() {
    Parser parser;
    std::vector<std::string> tokens;
    std::string path = R"(R 1 20 "asdlkjs\dfasdfdsf\")";

    parser.clear();
    parser.set_string(path.c_str());
    parser.parse();

    tokens.clear();
    tokens = parser.get_tokens();

    for(const auto &t : tokens) {
        std::cout << "TOKENS: |" << t << "|" << std::endl;
    }

    return 0;
}
