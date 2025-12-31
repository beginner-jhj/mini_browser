#include "css/style_applier.h"
#include <QColor>
#include <QString>
#include "util_functions.h"

StyleApplier::StyleApplier()
{
    register_handlers();
}

void StyleApplier::register_handlers()
{
    m_handlers["color"] = [](QPainter &painter, QFont &font, const std::string &value)
    {
        painter.setPen(parse_color(value));
    };

    m_handlers["font-size"] = [](QPainter &painter, QFont &font, const std::string &value)
    {
        font.setPixelSize(parse_font_size(value));
    };
}

void StyleApplier::apply(QPainter &painter, QFont &font, const std::map<std::string, std::string> &styles)
{
    for (const auto &[property, value] : styles)
    {
        auto it = m_handlers.find(property);
        if (it != m_handlers.end())
        {
            it->second(painter, font, value);
        }
    }
}

QColor StyleApplier::parse_color(const std::string &color_value)
{
    if (color_value.size() <= 3)
    {
        return QColor(QString::fromStdString(color_value));
    }
    if (color_value.substr(0, 3) != "rgb")
    {
        return QColor(QString::fromStdString(color_value));
    }

    size_t pos = 0;
    std::vector<double> values;
    std::string value;
    while (pos < color_value.size())
    {
        char current = color_value[pos];

        if (std::isdigit(current) || current == '.')
        {
            value += current;
        }

        else if (current == ',' || current == ')')
        {
            if (!value.empty())
            {
                values.push_back(std::stod(value));
            }
            value.clear();
        }

        ++pos;
    }

    if (values.size() == 4)
    {
        return QColor(values[0], values[1], values[2], values[3] * 255);
    }

    return QColor(values[0], values[1], values[2]);
}

int StyleApplier::parse_font_size(const std::string &value)
/* 1in = 96px = 2.54cm = 25.4mm
    96/2.54 is about 37.8px
*/
{
    bool is_keyword = value.find('-') != std::string::npos;

    if (is_keyword)
    {
        if (value == "xx-small")
        {
            return 9;
        }

        else if (value == "x-small")
        {
            return 10;
        }

        else if (value == "small")
        {
            return 13;
        }

        else if (value == "medium")
        {
            return 16;
        }

        else if (value == "large")
        {
            return 18;
        }

        else if (value == "x-large")
        {
            return 24;
        }

        else if (value == "xx-large")
        {
            return 32;
        }

        else
        {
            return 16;
        }
    }
    else
    {
        std::string unit = value.substr(value.size() - 2);
        try
        {
            double num_value = std::stod(value.substr(0, value.size() - 2));

            if (unit == "px" || unit == "pt")
            {
                return std::round(num_value);
            }

            else if (unit == "cm")
            {
                return std::round(num_value * 37.8);
            }

            else if (unit == "mm")
            {
                return std::round(num_value * 3.78);
            }

            else if (unit == "in")
            {
                return std::round(num_value * 96);
            }

            else
            {
                return 16;
            }
        }
        catch (...)
        {
            return 16;
        }
    }
}