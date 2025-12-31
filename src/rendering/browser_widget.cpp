#include "rendering/browser_widget.h"
#include <iostream>

BrowserWidget::BrowserWidget(QWidget *parent) : QWidget(parent), m_root(nullptr) {};

void BrowserWidget::set_document(std::shared_ptr<Node> root)
{
    m_root = root;
    update();
}

void BrowserWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    if (m_root != nullptr)
    {

        draw_node(painter, m_root, MARGIN_LEFT, 0);
    }
}

std::pair<int, int> BrowserWidget::draw_node(QPainter &painter, std::shared_ptr<Node> node, int x, int y)
{

    int current_y = y;
    int current_x = x;
    int display_width = this->width();
    for (auto &child : node->get_children())
    {
        if (child->get_type() == NODE_TYPE::TEXT)
        {
            QString text_content = QString::fromStdString(child->get_text_content());

            QFontMetrics fm(painter.font());
            int line_height = fm.height();

            QStringList words = text_content.split(" ");
            for (const QString &word : words)
            {
                QString word_with_space = word + " ";

                int word_width = fm.horizontalAdvance(word_with_space);
                if (current_x + word_width > display_width - MARGIN_LEFT)
                {
                    current_y += line_height;
                    current_x = x;
                }

                painter.drawText(current_x, current_y + line_height, word_with_space);
                current_x += word_width;
            }
        }
        else
        {
            std::string tag = child->get_tag_name();
            DISPLAY_TYPE display_type = child->get_display_type();

            if (display_type == DISPLAY_TYPE::BLOCK)
            {
                if (current_x > x)
                {
                    QFontMetrics fm(painter.font());

                    current_y += fm.height();
                    current_x = x;
                }

                QFont font = painter.font();

                apply_element_style(painter, child);
                QFontMetrics fm(painter.font());

                int child_x = x;

                if (tag == "ol" || tag == "ul")
                {
                    child_x += LIST_INDENT;
                }

                else if (tag == "li")
                {
                    painter.drawText(child_x, current_y + fm.height(), "•");
                    child_x += 15;
                }

                auto [new_x, new_y] = draw_node(painter, child, child_x, current_y);

                current_y = new_y + fm.height() / 2;
                current_x = x;

                reset_style(painter);
            }
            else
            {
                apply_element_style(painter, child);

                auto [new_x, new_y] = draw_node(painter, child, current_x, current_y);
                current_x = new_x;
                current_y = new_y;

                reset_style(painter);
            }
        }
    }
    return {current_x, current_y};
}

void BrowserWidget::apply_element_style(QPainter &painter, std::shared_ptr<Node> node)
{
    QFont font = painter.font();
    std::string tag = node->get_tag_name();

    if (tag == "h1")
    {
        font.setPointSize(FONT_SIZE_H1);
        font.setBold(true);
    }
    else if (tag == "h2")
    {
        font.setPointSize(FONT_SIZE_H2);
        font.setBold(true);
    }
    else if (tag == "p")
    {
        font.setPointSize(FONT_SIZE_P);
        font.setBold(false);
    }
    else if (tag == "strong")
    {
        font.setBold(true);
    }
    else if (tag == "em")
    {
        font.setItalic(true);
    }
    else if (tag == "strong")
    {
        font.setBold(true);
    }
    else if (tag == "em")
    {
        font.setItalic(true);
    }
    else if (tag == "a")
    {
        painter.setPen(QColor("#0066cc"));
        font.setUnderline(true);
    }

    m_style_applier.apply(painter, font, node->get_all_styles());
    painter.setFont(font);
}

void BrowserWidget::reset_style(QPainter &painter)
{
    QFont font = painter.font();
    font.setPointSize(FONT_SIZE_P);
    font.setBold(false);
    font.setItalic(false);
    font.setUnderline(false);
    painter.setFont(font);
    painter.setPen(QColor("#000000"));
}