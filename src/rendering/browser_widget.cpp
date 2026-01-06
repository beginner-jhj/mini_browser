#include "rendering/browser_widget.h"
#include "css/css_parser.h"
#include "css/apply_style.h"
#include <iostream>
BrowserWidget::BrowserWidget(QWidget *parent) : QWidget(parent), m_root(nullptr) {};

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
        std::string css = extract_stylesheets(m_root);
        std::string final_css = ua_css + "/n" + css;
        m_cssom = create_cssom(final_css);

        apply_style(m_root, m_cssom);
    }

    update();
}

void BrowserWidget::paint_layout(QPainter &painter, const LayoutBox &box, float offset_x, float offset_y, const LayoutBox *parent_box)
{
    float abs_x = offset_x + box.x;
    float abs_y = offset_y + box.y;

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
        std::string text = box.node->get_text_content();
        float baseline_y = abs_y + metrics.ascent();

        float text_x = abs_x;

        if (parent_box)
        {
            float text_width = metrics.horizontalAdvance(QString::fromStdString(text));

            if (parent_box->style.text_align == TextAlign::Center)
            {
                text_x = offset_x + (parent_box->width - text_width) / 2;
            }

            else if (parent_box->style.text_align == TextAlign::Right)
            {
                text_x = offset_x + parent_box->width - text_width;
            }
        }

        painter.drawText(text_x, baseline_y, QString::fromStdString(text));
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

void BrowserWidget::paintEvent(QPaintEvent *event)
{
    if (!m_root)
        return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.fillRect(rect(), Qt::white);

    int viewport_width = this->width();
    LineState line(viewport_width);
    LayoutBox layout = create_layout_tree(m_root, viewport_width, line);

    paint_layout(painter, layout, 0, 0);
}