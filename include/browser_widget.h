#pragma once
#include <QWidget>
#include <QPainter>
#include <QPaintEvent>
#include <memory>
#include "node.h"

class BrowserWidget:public QWidget{
    private:
        std::shared_ptr<Node> m_root;

        // Returns next Y position after drawing
    int draw_node(QPainter& painter, std::shared_ptr<Node> node, int x, int y);
    protected:
        void paintEvent(QPaintEvent* event) override;

    public:
        explicit BrowserWidget(QWidget* parent=nullptr);

        void set_document(std::shared_ptr<Node> root);
};