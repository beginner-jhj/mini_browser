#include "css/layout_tree.h"
#include <QDebug>
#include "util_functions.h"

LayoutBox create_layout_tree(
    std::shared_ptr<Node> root,
    float parent_width,
    LineState &line, int current_screen_width)
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
            // qDebug() << "Assigned width: " << box.style.width;
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
            LayoutBox child_box = create_layout_tree(child, child_parent_width, line, current_screen_width);

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
        std::vector<std::string> words = split_into_words(text);

        for (auto &word : words)
        {
            int word_width = metrics.horizontalAdvance(QString::fromStdString(word));
            int word_height = metrics.height();

            // qDebug() << "word_text: " << word;
            // qDebug() << "line.current_x + word_width = " << line.current_x << " + " << word_width << " = " << line.current_x +word_width;
            // qDebug() << "max_width: " << line.max_width;
            bool will_wrap = (line.current_x + word_width > line.max_width || line.current_x + word_width > current_screen_width) && line.current_x > 0;

            // qDebug() << "will_wrap: " << will_wrap;

            if (will_wrap)
            {
                line.current_x = line.padding_left;
                line.current_y += line.line_height;
                line.line_height = 0;
            }

            LayoutBox word_box;
            word_box.node = root;
            word_box.text = word;
            word_box.style = box.style;
            word_box.x = line.current_x;
            word_box.y = line.current_y;
            word_box.width = word_width;
            word_box.height = word_height;

            box.children.push_back(word_box);

            line.current_x += word_width;

            float effective_line_height = std::max(static_cast<float>(word_height), box.style.line_height);
            if (effective_line_height > line.line_height)
            {
                line.line_height = effective_line_height;
            }
        }

        box.x = box.children[0].x;
        box.y = box.children[0].y;
        box.width = line.current_x - box.x;
        box.height = line.line_height;

        return box;
    }

    if (box.style.display == DISPLAY_TYPE::INLINE)
    {
        float start_x = line.current_x;
        float start_y = line.current_y;

        for (auto child : root->get_children())
        {
            LayoutBox child_box = create_layout_tree(child, parent_width, line, current_screen_width);
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