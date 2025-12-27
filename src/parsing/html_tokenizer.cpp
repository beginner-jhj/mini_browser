#include "html_tokenizer.h"

std::map<std::string, std::string> parse_attrubute(const std::string &to_parse)
{
    std::map<std::string, std::string> attrs;
    size_t pos = 0;
    while (pos < to_parse.size())
    {
        while (pos < to_parse.size() && to_parse[pos] == ' ')
        {
            pos++;
        }

        size_t equal_sign_pos = to_parse.find("=", pos);

        std::string attribute_name = to_parse.substr(pos, equal_sign_pos - pos);

        pos = equal_sign_pos+1;

        while (pos < to_parse.size() && to_parse[pos] == ' ')
        {
            pos++;
        }

        std::string attribute_value{};
        if (to_parse[pos] == '"' || to_parse[pos] == '\'')
        {
            char quote = to_parse[pos];
            pos++;
            size_t closing_quote_pos = to_parse.find(quote,pos);
            attribute_value = to_parse.substr(pos, closing_quote_pos - pos);
            pos = closing_quote_pos + 1;
        }
        else
        {
        }

        if (!attribute_value.empty())
        {
            attrs[attribute_name] = attribute_value;
        }
    }

    return attrs;
}

std::vector<Token> tokenize(const std::string &html)
{
    std::vector<Token> tokens;

    size_t pos = 0;
    while (pos < html.size())
    {
        if (html[pos] == '<')
        {
            size_t end_pos = html.find('>', pos);

            if (end_pos != std::string::npos)
            {
                if (html[pos + 1] == '/')
                {
                    tokens.push_back({TOKEN_TYPE::END_TAG, html.substr(pos + 2, end_pos - pos - 2), {}});
                }
                else
                {
                    std::string full_tag = html.substr(pos + 1, end_pos - pos - 1);
                    size_t space_pos = full_tag.find(' ');

                    std::string tag_name;
                    std::map<std::string, std::string> attrs;

                    if (space_pos != std::string::npos)
                    {
                        tag_name = full_tag.substr(0, space_pos);
                        std::string attr_string = full_tag.substr(space_pos + 1);
                        attrs = parse_attrubute(attr_string);
                    }
                    else
                    {
                        tag_name = full_tag;
                    }
                    tokens.push_back({TOKEN_TYPE::START_TAG, tag_name, attrs});
                }
                pos = end_pos + 1;
            }
            else
            {
                throw std::runtime_error("INVALIDE HTML: NO CLOSING TAG");
            }
        }
        else
        {
            size_t end_pos = html.find('<', pos);
            if (!html.substr(pos, end_pos - pos).empty())
            {
                tokens.push_back({TOKEN_TYPE::TEXT, html.substr(pos, end_pos - pos)});
            }
            pos = end_pos;
        }
    }
    return tokens;
}
