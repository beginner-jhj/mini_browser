#include "rendering/browser_widget.h"
#include "css/css_parser.h"
#include "css/apply_style.h"
#include <iostream>
#include <QDebug>
BrowserWidget::BrowserWidget(QWidget *parent) : QWidget(parent), m_root(nullptr) {}

void BrowserWidget::set_document(std::shared_ptr<Node> root)
{
    m_root = root;
    if (m_root)
    {

        std::string ua_css = R"(
            /* Hidden elements */
            head { display: none; }
            style { display: none; }
            script { display: none; }
            meta { display: none; }
            link { display: none; }
            title { display: none; }
            
            /* Block elements */
            html { display: block; }
            body { display: block; }
            div { display: block; }
            p { display: block; }
            h1 { display: block; }
            h2 { display: block; }
            ul { display: block; }
            ol { display: block; }
            li { display: block; }
            
            /* Inline elements */
            span { display: inline; }
            a { display: inline; }
            strong { display: inline; }
            em { display: inline; }
        )";
        std::string author_css = extract_stylesheets(m_root);

        std::string combined_css = ua_css + "\n" + author_css;
        m_cssom = create_cssom(combined_css);

        apply_style(m_root, m_cssom);
    }

    update();
}

void BrowserWidget::paint_layout(QPainter &painter, const LayoutBox &box, float offset_x, float offset_y, const LayoutBox *parent_box)
{
    float abs_x = offset_x + box.x;
    float abs_y = offset_y + box.y;

    float previous_opacity = painter.opacity();
    painter.setOpacity(previous_opacity * box.style.opacity);

    if (box.node->get_type() == NODE_TYPE::ELEMENT)
    {
        if (box.style.background_color != QColor("transparent"))
        {
            painter.fillRect(
                abs_x, abs_y,
                box.width, box.height,
                box.style.background_color);
        }

        if (box.style.border_width > 0)
        {
            QPen pen;
            pen.setColor(box.style.border_color);
            pen.setStyle(box.style.border_style);
            pen.setWidthF(box.style.border_width);

            painter.setPen(pen);
            painter.setBrush(Qt::NoBrush);

            painter.drawRect(abs_x, abs_y, box.width, box.height);
        }
    }

    if (box.node->get_type() == NODE_TYPE::TEXT)
    {
        QFont ft = box.style.to_font();
        painter.setFont(ft);
        painter.setPen(box.style.color);

        QFontMetrics metrics(ft);

        float offset_adjust = 0;

        if (parent_box)
        {
            float total_width = 0;
            for (const auto &word_box : box.children)
            {
                total_width += word_box.width;
            }

            if (parent_box->style.text_align == TextAlign::Center)
            {
                offset_adjust = (parent_box->width - total_width) / 2;
            }
            else if (parent_box->style.text_align == TextAlign::Right)
            {
                offset_adjust = parent_box->width - total_width;
            }
        }

        for (const auto &word_box : box.children)
        {
            float word_abs_x = offset_x + word_box.x + offset_adjust;
            float word_abs_y = offset_y + word_box.y;
            float baseline_y = word_abs_y + metrics.ascent();

            painter.drawText(word_abs_x, baseline_y,
                             QString::fromStdString(word_box.text));

            if (word_box.style.text_decoration != TextDecoration::None)
            {
                QPen decoration_pen(box.style.color);
                decoration_pen.setWidth(1);
                painter.setPen(decoration_pen);
                float decoration_y = 0;
                switch (box.style.text_decoration)
                {
                case TextDecoration::UnderLine:
                    decoration_y = baseline_y + 1;
                    break;
                case TextDecoration::LineThrough:
                    decoration_y = word_abs_y + metrics.ascent() / 2;
                    break;
                default:
                    decoration_y = word_abs_y;
                    break;
                }
                painter.drawLine(word_abs_x, decoration_y, word_abs_x + word_box.width, decoration_y);
                painter.setPen(box.style.color);
            }
        }

        return;
    }

    for (const auto &child : box.children)
    {
        paint_layout(painter, child, abs_x, abs_y, &box);
    }
}

// void print_layout_debug(const LayoutBox& box, int depth) {
//     std::string indent(depth * 2, ' ');
//     std::string tag = box.node->get_tag_name();

//     if (box.node->get_type() == NODE_TYPE::TEXT) {
//         std::string text = box.node->get_text_content();
//         if (text.size() > 15) text = text.substr(0, 15) + "...";
//         tag = "TEXT: [" + text + "]";
//     }

//     std::cout << indent << tag
//               << " at (" << box.x << ", " << box.y << ")"
//               << " size " << box.width << "x" << box.height
//               << std::endl;

//     for (const auto& child : box.children) {
//         print_layout_debug(child, depth + 1);
//     }
// }

float calculate_content_width(const LayoutBox &box)
{
    float max_right = box.x + box.width;

    for (const auto &child : box.children)
    {
        float child_right = child.x + calculate_content_width(child);
        if (child_right > max_right)
        {
            max_right = child_right;
        }
    }

    return max_right;
}

void BrowserWidget::paintEvent(QPaintEvent *event)
{
    if (!m_root)
        return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.fillRect(rect(), Qt::white);

    QWidget *scrollArea = parentWidget();
    int viewport_width = scrollArea ? scrollArea->width() : this->width();

    LineState line(viewport_width);
    LayoutBox layout = create_layout_tree(m_root, viewport_width, line);

    paint_layout(painter, layout, 0, 0);

    float content_width = calculate_content_width(layout);

    int min_width = std::max(static_cast<float>(viewport_width), content_width);

    setMinimumSize(min_width, layout.height);
}