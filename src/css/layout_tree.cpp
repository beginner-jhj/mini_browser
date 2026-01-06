#include "css/layout_tree.h"
#include <QDebug>

LayoutBox create_layout_tree(
    std::shared_ptr<Node> root,
    float parent_width,
    LineState &line)
{
    LayoutBox box;
    box.node = root;
    box.style = root->get_all_styles();

    if (box.style.display == DISPLAY_TYPE::NONE)
    {
        return box;
    }

    if (root->get_type() == NODE_TYPE::TEXT)
    {
        std::string text = root->get_text_content();
        text.erase(0, text.find_first_not_of(" \t\n\r"));
        text.erase(text.find_last_not_of(" \t\n\r") + 1);
        if (text.empty())
        {
            return box;
        }
    }

    if (box.style.display == DISPLAY_TYPE::BLOCK)
    {
        if (box.style.width > 0)
        {
            box.width = box.style.width;
        }
        else
        {
            box.width = parent_width - box.style.margin_left - box.style.margin_right;
        }

        line.current_x = box.style.padding_left;
        line.current_y = box.style.padding_top;
        line.line_height = 0;
        line.max_width = box.width - box.style.padding_right - box.style.padding_left;
        line.padding_left = box.style.padding_left;

        float content_y = box.style.padding_top;
        for (auto child : root->get_children())
        {

            float child_parent_width = box.width -
                                       box.style.padding_left -
                                       box.style.padding_right;
            LayoutBox child_box = create_layout_tree(child, child_parent_width, line);

            if (child_box.style.display == DISPLAY_TYPE::BLOCK)
            {
                child_box.x = child_box.style.margin_left + box.style.padding_left;
                child_box.y = content_y + child_box.style.margin_top;
                content_y += child_box.height + child_box.style.margin_top + child_box.style.margin_bottom;

                line.current_x = box.style.padding_left;
                line.current_y = content_y;
                line.line_height = 0;
            }
            else
            {
                float inline_bottom = line.current_y + line.line_height;
                if (inline_bottom > content_y)
                {
                    content_y = inline_bottom;
                }
            }

            box.children.push_back(child_box);
        }

        if (box.style.height > 0)
        {
            box.height = box.style.height;
        }
        else
        {
            box.height = content_y + box.style.padding_bottom;
        }

        return box;
    }

    if (root->get_type() == NODE_TYPE::TEXT)
    {
        QFont font = box.style.to_font();
        QFontMetrics metrics(font);

        std::string text = root->get_text_content();
        int text_width = metrics.horizontalAdvance(
            QString::fromStdString(text));
        int text_height = metrics.height();

        if (line.current_x + text_width > line.max_width && line.current_x > 0)
        {
            line.current_x = line.padding_left;
            line.current_y += line.line_height;
            line.line_height = 0;
        }

        box.x = line.current_x;
        box.y = line.current_y;
        box.width = text_width;
        box.height = text_height;

        line.current_x += text_width;
        if (text_height > line.line_height)
        {
            line.line_height = text_height;
        }

        return box;
    }

    if (box.style.display == DISPLAY_TYPE::INLINE)
    {
        float start_x = line.current_x;
        float start_y = line.current_y;

        for (auto child : root->get_children())
        {
            LayoutBox child_box = create_layout_tree(child, parent_width, line);
            child_box.x -= start_x;
            child_box.y -= start_y;
            box.children.push_back(child_box);
        }

        float end_x = line.current_x;
        float max_height = line.line_height;

        box.x = start_x;
        box.y = start_y;
        box.width = end_x - start_x;
        box.height = max_height;

        return box;
    }

    return box;
}