#pragma once
#include "token.h"
#include <string>
#include <vector>

std::vector<Token> tokenize(const std::string& html);
std::map<std::string, std::string> parse_attrubute(const std::string& to_parse);