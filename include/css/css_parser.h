#pragma once
#include <unordered_map>
#include <string_view>

std::unordered_map<std::string, std::string> parse_inline_style(std::string_view style_string);