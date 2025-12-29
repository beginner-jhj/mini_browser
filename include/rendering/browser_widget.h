#pragma once
#include <QWidget>
#include <QPainter>
#include <QPaintEvent>
#include <memory>
#include <utility>
#include "html/node.h"

class BrowserWidget : public QWidget
{
private:
    static constexpr int LINE_HEIGHT = 20;
    static constexpr int MARGIN_LEFT = 10;
    static constexpr int FONT_SIZE_H1 = 32;
    static constexpr int FONT_SIZE_H2 = 24;
    static constexpr int FONT_SIZE_P = 16;
    static constexpr int LIST_INDENT = 30;
    std::shared_ptr<Node> m_root;

    // Returns next Y position after drawing
    std::pair<int, int> draw_node(QPainter &painter, std::shared_ptr<Node> node, int x, int y);
    void apply_element_style(QPainter &painter, std::shared_ptr<Node> node);
    void reset_style(QPainter &painter);

protected:
    void paintEvent(QPaintEvent *event) override;

public:
    explicit BrowserWidget(QWidget *parent = nullptr);

    void set_document(std::shared_ptr<Node> root);
};