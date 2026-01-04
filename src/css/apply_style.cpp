#include "css/apply_style.h"
#include "css/css_parser.h"
#include <queue>

void apply_style(std::shared_ptr<Node> node, CSSOM &cssom, bool is_root)
{
    static bool setters_initialized = false;
    if (!setters_initialized)
    {
        ComputedStyle::init_setters();
        setters_initialized = true;
    }
    std::queue<std::shared_ptr<Node>> q;
    q.push(node);

    ComputedStyle parent_style;

    while (!q.empty())
    {
        auto current_node = q.front();
        q.pop();

        if (!is_root)
        {
            current_node->set_style("color", parent_style.inherit_color());

            current_node->set_style("font-size", parent_style.inherit_font_size());

            current_node->set_style("font-weight", parent_style.inherit_font_weight());

            current_node->set_style("font-style", parent_style.inherit_font_style());

            current_node->set_style("font-family", parent_style.inherit_font_family());

            current_node->set_style("line-height", parent_style.inherit_line_height());

            current_node->set_style("text-align", parent_style.inherit_text_align());

            current_node->set_style("visibility", parent_style.inherit_visibility());
        }

        parent_style = current_node->get_all_styles();
        is_root = false;

        auto matched_rules = cssom.matching_rules(current_node);
        for (const auto &rule : matched_rules)
        {
            for (const auto &decl : rule.decelarations)
            {
                current_node->set_style(decl.property, decl.value);
            }
        }

        std::string inline_style = current_node->get_attribute("style");

        for(const auto& [property, value]:parse_inline_style(inline_style)){
            current_node->set_style(property, value);
        }

        for (auto child : current_node->get_children())
        {
            q.push(child);
        }
    }
}