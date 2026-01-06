#pragma once
#include <memory>
#include <vector>
#include <QFont>
#include <QFontMetrics>
#include "html/node.h"
#include "css/computed_style.h"

struct LineState {
    float current_x = 0;
    float current_y = 0;
    float line_height = 0;
    float max_width = 0;
    float padding_left = 0;
    
    LineState(float width = 0) : max_width(width) {}
};

struct LayoutBox {
    std::shared_ptr<Node> node;
    ComputedStyle style;
    
    float x = 0;
    float y = 0;
    float width = 0;
    float height = 0;
    
    std::vector<LayoutBox> children;
};

LayoutBox create_layout_tree(
    std::shared_ptr<Node> root,
    float parent_width,
    LineState& line
);