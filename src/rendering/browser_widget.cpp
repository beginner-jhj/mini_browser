#include "browser_widget.h"

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

        draw_node(painter, m_root, 100, 0);
    }
}

int BrowserWidget::draw_node(QPainter &painter, std::shared_ptr<Node> node, int x, int y)
{

    int current_y = y;
    for (auto &child : node->get_children())
    {
        if (child->get_type() == NODE_TYPE::TEXT)
        {
            QString text_content = QString::fromStdString(child->get_text_content());
            painter.drawText(x, current_y+20, text_content);
            current_y+=20;
        }
        else
        {
            current_y = draw_node(painter, child, x, current_y);
        }
    }
    return current_y;
}