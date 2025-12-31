#include "css/css_parser.h"
#include "util_functions.h"
#include <iostream>
#include <cctype>

std::unordered_map<std::string, std::string> parse_inline_style(std::string_view style_string)
{
    std::unordered_map<std::string, std::string> result;
    if (style_string.empty())
        return result;
    std::string name, value;
    bool is_value = false;
    int paren_depth = 0;
    bool is_in_quote = false;
    size_t pos = 0;

    while (pos < style_string.size())
    {
        char current = style_string[pos];

        if (current == ':' && !is_value)
        {
            is_value = true;
        }
        else if (current == ';' && paren_depth == 0 && !is_in_quote)
        {
            if (!name.empty() && !value.empty())
            {
                result[name] = trim_copy(value);
            }
            name.clear();
            value.clear();
            is_value = false;
        }
        else if (std::isspace(current) && !is_value)
        {
            ++pos;
            continue;
        }
        else
        {
            if (!is_value)
            {
                name += std::tolower(static_cast<unsigned char>(current));
            }
            else
            {
                if (current == '(')
                {
                    ++paren_depth;
                }

                else if (current == ')')
                {
                    --paren_depth;
                }

                if (current == '"' || current == '\'')
                {
                    is_in_quote = !is_in_quote;
                }
                value += paren_depth > 0 || is_in_quote ? current : std::tolower(static_cast<unsigned char>(current));
            }
        }

        ++pos;
    }

    if (!name.empty() && !value.empty())
    {
        result[name] = trim_copy(value);
    }

    return result;
}