#pragma once
#include <QWidget>
#include <QPainter>
#include <QPaintEvent>
#include <memory>
#include "html/node.h"
#include "css/cssom.h"
#include "css/layout_tree.h"

class BrowserWidget : public QWidget
{
private:
    std::shared_ptr<Node> m_root;
    CSSOM m_cssom;
    int m_current_screen_width;

    void paint_layout(QPainter& painter, const LayoutBox& box, float offset_x, float offset_y, const LayoutBox *parent_box=nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

public:
    explicit BrowserWidget(QWidget *parent = nullptr);
    void set_document(std::shared_ptr<Node> root);
};