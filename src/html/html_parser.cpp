#include "html/html_parser.h"
#include "css/css_parser.h"
#include "util_functions.h"
#include <set>

/**
 * \brief Parses a sequence of HTML tokens into a DOM tree structure.
 *
 * Processes a vector of tokens and builds a hierarchical Node tree representation
 * of the HTML document. Handles element nesting, text nodes, and void elements
 * that do not require closing tags.
 *
 * \param tokens A vector of TOKEN objects representing the tokenized HTML.
 * \return A shared pointer to the root Node of the parsed DOM tree.
 */
std::shared_ptr<NODE> parse(const std::vector<TOKEN> &tokens)
{
    std::set<std::string> void_elements = {"meta", "link", "img", "br", "hr", "input"};
    std::vector<std::shared_ptr<NODE>> stack;
    std::shared_ptr<NODE> root = nullptr;
    for (auto &token : tokens)
    {
        if (token.type == TOKEN_TYPE::TEXT)
        {
            std::string text = token.value;
            text.erase(0, text.find_first_not_of(" \t\n\r"));
            text.erase(text.find_last_not_of(" \t\n\r") + 1);

            if (text.empty())
            {
                continue;
            }
        }
        if (token.type == TOKEN_TYPE::TEXT && stack.empty())
        {
            std::shared_ptr<NODE> html = std::make_shared<NODE>(NODE_TYPE::ELEMENT, "html");
            std::shared_ptr<NODE> body = std::make_shared<NODE>(NODE_TYPE::ELEMENT, "body");

            std::shared_ptr<NODE> text_node = create_node(token);

            body->add_child(text_node);
            html->add_child(body);

            root = html;

            stack.push_back(html);
            stack.push_back(body);

            continue;
        }

        if (token.type == TOKEN_TYPE::START_TAG)
        {
            bool is_void_token = void_elements.find(token.value) != void_elements.end();
            std::shared_ptr<NODE> new_node = create_node(token);
            if (!stack.empty())
            {
                stack.back()->add_child(new_node);
            }
            else
            {
                root = new_node;
            }
            if(!is_void_token){
                stack.push_back(new_node);
            }
            continue;
        }

        else if (token.type == TOKEN_TYPE::TEXT)
        {
            if (!stack.empty())
            {
                std::shared_ptr<NODE> parent = stack.back();
                parent->add_child(create_node(token));
            }
            continue;
        }

        else
        {
            if(!stack.empty()){
                stack.pop_back();
            }
        }
    }

    return root;
}

/**
 * \brief Creates a Node object from a TOKEN.
 *
 * Converts a single TOKEN into a corresponding Node. For text tokens, creates
 * a TEXT type node. For element tokens, creates an ELEMENT type node and
 * populates its attributes from the token's attribute map.
 *
 * \param token The TOKEN object to convert into a Node.
 * \return A shared pointer to the newly created Node.
 */
std::shared_ptr<NODE> create_node(const TOKEN &token)
{
    if (token.type == TOKEN_TYPE::TEXT)
    {
        return std::make_shared<NODE>(NODE_TYPE::TEXT, token.value);
    }

    auto node = std::make_shared<NODE>(NODE_TYPE::ELEMENT, token.value);

    if (!token.attributes.empty())
    {
        for (auto [name, value] : token.attributes)
        {
            node->set_attribute(name, value);
        }
    }
    return node;
}