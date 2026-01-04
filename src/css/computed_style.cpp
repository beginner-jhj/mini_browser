#include "css/computed_style.h"
#include "html/node.h"

std::unordered_map<std::string, ComputedStyle::Setter> ComputedStyle::setters;

static bool initialized = false;

void ComputedStyle::init_setters()
{
    if (initialized)
        return;
    initialized = true;
    setters["color"] = [](ComputedStyle &s, const std::string &value)
    {
        s.color = parse_color(value);
    };

    setters["font-size"] = [](ComputedStyle &s, const std::string &value)
    {
        s.font_size = parse_font_size(value);
    };

    setters["font-weight"] = [](ComputedStyle &s, const std::string &value)
    {
        try
        {
            if (value == "normal")
            {
                s.font_weight = QFont::Normal;
                return;
            }

            if (value == "bold")
            {
                s.font_weight = QFont::Bold;
                return;
            }
            int weight = std::stoi(value);
            int adjusted_weight;
            if (weight >= 100 && weight < 1000)
            {
                adjusted_weight = std::round(weight / 100) * 100;

                if (adjusted_weight <= 100)
                {
                    s.font_weight = QFont::Thin;
                }

                else if (adjusted_weight == 200)
                {
                    s.font_weight = QFont::ExtraLight;
                }

                else if (adjusted_weight == 300)
                {
                    s.font_weight = QFont::Light;
                }

                else if (adjusted_weight == 400)
                {
                    s.font_weight = QFont::Normal;
                }

                else if (adjusted_weight == 500)
                {
                    s.font_weight = QFont::Medium;
                }

                else if (adjusted_weight == 600)
                {
                    s.font_weight = QFont::DemiBold;
                }

                else if (adjusted_weight == 700)
                {
                    s.font_weight = QFont::Bold;
                }

                else if (adjusted_weight == 800)
                {
                    s.font_weight = QFont::ExtraBold;
                }

                else if (adjusted_weight >= 900)
                {
                    s.font_weight = QFont::Black;
                }
            }
            else
            {
                s.font_weight = QFont::Normal;
            }
        }
        catch (...)
        {
            s.font_weight = QFont::Normal;
        }
    };

    setters["font-style"] = [](ComputedStyle &s, const std::string &value)
    {
        s.font_style = value;
    };

    setters["font-family"] = [](ComputedStyle &s, const std::string &value)
    {
        s.font_family = QString::fromStdString(value);
    };

    setters["background-color"] = [](ComputedStyle &s, const std::string &value)
    {
        s.background_color = parse_color(value);
    };

    setters["width"] = [](ComputedStyle &s, const std::string &value)
    {
        s.width = parse_string_to_float(value, -1);
    };

    setters["height"] = [](ComputedStyle &s, const std::string &value)
    {
        s.height = parse_string_to_float(value, -1);
    };

    setters["margin-top"] = [](ComputedStyle &s, const std::string &value)
    {
        s.margin_top = parse_string_to_float(value, 0);
    };

    setters["margin-bottom"] = [](ComputedStyle &s, const std::string &value)
    {
        s.margin_bottom = parse_string_to_float(value, 0);
    };

    setters["margin-left"] = [](ComputedStyle &s, const std::string &value)
    {
        s.margin_left = parse_string_to_float(value, 0);
    };

    setters["margin-right"] = [](ComputedStyle &s, const std::string &value)
    {
        s.margin_right = parse_string_to_float(value, 0);
    };

    setters["padding-top"] = [](ComputedStyle &s, const std::string &value)
    {
        s.padding_top = parse_string_to_float(value, 0);
    };

    setters["padding-botom"] = [](ComputedStyle &s, const std::string &value)
    {
        s.padding_bottom = parse_string_to_float(value, 0);
    };

    setters["padding-left"] = [](ComputedStyle &s, const std::string &value)
    {
        s.padding_left = parse_string_to_float(value, 0);
    };

    setters["padding_right"] = [](ComputedStyle &s, const std::string &value)
    {
        s.padding_right = parse_string_to_float(value, 0);
    };

    setters["border-width"] = [](ComputedStyle &s, const std::string &value)
    {
        s.border_width = parse_string_to_float(value, 0);
    };

    setters["border-color"] = [](ComputedStyle &s, const std::string &value)
    {
        s.border_color = parse_color(value);
    };

    setters["border-style"] = [](ComputedStyle &s, const std::string &value)
    {
        if (value == "solid")
        {
            s.border_style = Qt::SolidLine;
        }

        else if (value == "dashed")
        {
            s.border_style = Qt::DashLine;
        }

        else if (value == "dotted")
        {
            s.border_style = Qt::DotLine;
        }

        else
        {
            s.border_style = Qt::NoPen;
        }
    };

    setters["display"] = [](ComputedStyle &s, const std::string &value)
    {
        if (value == "inline")
        {
            s.display = DISPLAY_TYPE::INLINE;
        }

        else if (value == "block")
        {
            s.display = DISPLAY_TYPE::BLOCK;
        }

        else
        {
            s.display = DISPLAY_TYPE::NONE;
        }
    };

    setters["box-sizing"] = [](ComputedStyle &s, const std::string &value)
    {
        if (value == "border-box")
        {
            s.box_sizing = BoxSizing::BorderBox;
        }
        else
        {
            s.box_sizing = BoxSizing::ContentBox;
        }
    };

    setters["text-align"] = [](ComputedStyle &s, const std::string &value)
    {
        if (value == "center")
        {
            s.text_align = TextAlign::Center;
        }

        else if (value == "right")
        {
            s.text_align = TextAlign::Right;
        }

        else if (value == "justify")
        {
            s.text_align = TextAlign::Justify;
        }

        else
        {
            s.text_align = TextAlign::Left;
        }
    };

    setters["line-height"] = [](ComputedStyle &s, const std::string &value)
    {
        s.line_height = parse_string_to_float(value, 16 * 1.5);
    };

    setters["visibility"] = [](ComputedStyle &s, const std::string &value)
    {
        if (value == "hidden" || value == "collapse")
        {
            s.visibility = false;
        }
        else
        {
            s.visibility = true;
        }
    };
}

QColor ComputedStyle::parse_color(const std::string &color_value)
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

int ComputedStyle::parse_font_size(const std::string &value)
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

float ComputedStyle::parse_string_to_float(const std::string &value, const float default_value)
{
    try
    {
        float num_value = std::stof(value);
        return num_value;
    }
    catch (...)
    {
        return default_value;
    }
}

std::string ComputedStyle::inherit_color() const
{
    QString color_hex_name = color.name();
    return color_hex_name.toStdString();
}

std::string ComputedStyle::inherit_font_size() const
{
    return std::to_string(font_size);
}

std::string ComputedStyle::inherit_font_weight() const
{
    int weight = font_weight;
    if (weight >= 100)
    {
        return std::to_string(weight);
    }

    else
    {
        if (weight < 10)
        {
            return std::to_string(weight * 100);
        }

        return std::to_string(weight * 10);
    }
}

std::string ComputedStyle::inherit_font_style() const
{
    return font_style;
}

std::string ComputedStyle::inherit_font_family() const
{
    return font_family.toStdString();
}

std::string ComputedStyle::inherit_line_height() const
{
    return std::to_string(line_height);
}

std::string ComputedStyle::inherit_text_align() const
{
    if (text_align == TextAlign::Center)
    {
        return "center";
    }

    else if (text_align == TextAlign::Justify)
    {
        return "justify";
    }

    else if (text_align == TextAlign::Right)
    {
        return "right";
    }

    else
    {
        return "left";
    }
}

std::string ComputedStyle::inherit_visibility() const {
    if(!visibility){
        return "hidden";
    }else{
        return "visible";
    }
}
