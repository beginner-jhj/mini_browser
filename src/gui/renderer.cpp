#include "gui/renderer.h"
#include "css/css_parser.h"
#include "css/apply_style.h"
#include <QScrollArea>
#include <QScrollBar>
#include <queue>

Renderer::Renderer(QWidget *parent) : QWidget(parent), m_root(nullptr), m_viewport_height(0), m_viewport_width(0) {}

void Renderer::set_document(std::shared_ptr<Node> root)
{
    m_root = root;

    if (m_root)
    {
        std::string user_agent_css = R"(
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

        std::string combined_css = user_agent_css + "\n" + author_css;
        m_cssom = create_cssom(combined_css);

        apply_style(m_root, m_cssom);
    }
    update();
}

float calculate_content_width(const LayoutBox &box);

void Renderer::paintEvent(QPaintEvent *event)
{
    if (!m_root)
    {
        return;
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.fillRect(rect(), Qt::white);

    QWidget *scroll_area = parentWidget();
    int viewport_width = scroll_area ? scroll_area->width() : this->width();
    int viewport_height = scroll_area ? scroll_area->height() : this->height();

    LineState line(viewport_width);
    LayoutBox layout = create_layout_tree(m_root, viewport_width, line);

    paint_layout(painter, layout, 0, 0);

    float content_width = calculate_content_width(layout);

    int min_width = std::max(static_cast<float>(viewport_width), content_width);

    setMinimumSize(min_width, layout.height);
}

void Renderer::paint_layout(QPainter &painter, const LayoutBox &box, float offset_x, float offset_y, const LayoutBox *parent_box)
{
    float abs_x = offset_x + box.x;
    float abs_y = offset_y + box.y;

    float previous_opacity = painter.opacity();
    painter.setOpacity(previous_opacity * box.style.opacity);

    if (box.style.position == PositionType::Relative)
    {
        abs_x += box.style.left - box.style.right;
        abs_y += box.style.top - box.style.bottom;
    }

    if (box.node->get_type() == NODE_TYPE::ELEMENT)
    {
        if (box.style.background_color != QColor("trnasparent"))
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

            painter.drawText(word_abs_x, baseline_y, QString::fromStdString(word_box.text));

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
            }
        }

        painter.setPen(box.style.color);

        return;
    }

    for (const auto &child : box.children)
    {
        paint_layout(painter, child, abs_x, abs_y, &box);
    }

    for (const auto &abs_child : box.absolute_children)
    {
        if (abs_child.style.position == PositionType::Fixed)
        {
            paint_fixed(painter, abs_child);
        }

        else
        {
            paint_layout(painter, abs_child, abs_x, abs_y, &box);
        }
    }
}

void Renderer::paint_fixed(QPainter &painter, const LayoutBox &box)
{
    // QScrollArea -> ViewPort(hidden) -> renderer
    QScrollArea *scroll_area = qobject_cast<QScrollArea *>(parentWidget()->parentWidget());
    int scroll_x = 0;
    int scroll_y = 0;

    if (scroll_area)
    {
        scroll_x = scroll_area->horizontalScrollBar()->value();
        scroll_y = scroll_area->verticalScrollBar()->value();
    }

    float draw_x = 0, draw_y = 0;

    if (box.style.is_left_set)
    {
        draw_x = scroll_x + box.style.left;
    }

    else if (box.style.is_right_set)
    {
        draw_x = scroll_x + m_viewport_width - box.width - box.style.right;
    }
    else
    {
        draw_x = scroll_x;
    }

    if (box.style.is_top_set)
    {
        draw_y = scroll_y + box.style.top;
    }

    else if (box.style.is_bottom_set)
    {
        draw_y = scroll_y + m_viewport_height - box.height - box.style.top;
    }
    else
    {
        draw_y = scroll_y;
    }

    float previous_opacity = painter.opacity();
    painter.setOpacity(previous_opacity * box.style.opacity);

    if (box.style.background_color != QColor("transparent"))
    {
        painter.fillRect(draw_x, draw_y, box.width, box.height, box.style.background_color);
    }

    if (box.style.border_width > 0)
    {
        QPen pen;
        pen.setColor(box.style.border_color);
        pen.setStyle(box.style.border_style);
        pen.setWidthF(box.style.border_width);

        painter.setPen(pen);
        painter.setBrush(Qt::NoBrush);

        painter.drawRect(draw_x, draw_y, box.width, box.height);
    }

    for (const auto &child : box.children)
    {
        paint_layout(painter, child, draw_x, draw_y, &box);
    }

    painter.setOpacity(previous_opacity);
}

float calculate_content_width(const LayoutBox& root){
    float max_right = root.x + root.width;
    std::queue<LayoutBox> q;
    q.push(root);

    while(!q.empty()){
        auto current_box = q.front();
        q.pop();

        float current_right = current_box.x + current_box.width;
        if(current_right > max_right){
            max_right = current_right;
        }

        for(const auto &child : current_box.children){
            q.push(child);
        }
    }

    return max_right -  root.x;
}