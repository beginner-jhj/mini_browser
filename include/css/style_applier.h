#pragma once
#include <functional>
#include <QPainter>
#include <QFont>
#include <QColor>
#include <unordered_map>
#include "html/node.h"

class StyleApplier
{

public:
    using STYLE_HANDLER = std::function<void(QPainter &, QFont &, const std::string &)>;

    StyleApplier();

    void apply(QPainter& painter, QFont& font, const std::map<std::string, std::string>& styles);

private:
    std::unordered_map<std::string, STYLE_HANDLER> m_handlers;
    void register_handlers();

    static QColor parse_color(const std::string& color_value);

    static int parse_font_size(const std::string& value);
};
