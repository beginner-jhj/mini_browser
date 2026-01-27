#include "css/computed_style.h"
#include "html/node.h"
#include <QDebug>
#include <sstream>

std::unordered_map<std::string, COMPUTED_STYLE::Setter> COMPUTED_STYLE::setters;

static bool initialized = false;

// ============================================================================
// Enum Parser Helper Functions
// ============================================================================

/**
 * \brief Parses a string value to DISPLAY_TYPE enum.
 * 
 * \param value CSS display value (e.g., "block", "inline", "none")
 * \return Corresponding DISPLAY_TYPE enum value
 */
DISPLAY_TYPE COMPUTED_STYLE::parse_display_type(const std::string &value)
{
    if (value == "inline") return DISPLAY_TYPE::INLINE;
    if (value == "block") return DISPLAY_TYPE::BLOCK;
    return DISPLAY_TYPE::NONE;
}

/**
 * \brief Parses a string value to TEXT_ALIGN enum.
 * 
 * \param value CSS text-align value (e.g., "left", "center", "right", "justify")
 * \return Corresponding TEXT_ALIGN enum value
 */
TEXT_ALIGN COMPUTED_STYLE::parse_text_align(const std::string &value)
{
    if (value == "center") return TEXT_ALIGN::Center;
    if (value == "right") return TEXT_ALIGN::Right;
    if (value == "justify") return TEXT_ALIGN::Justify;
    return TEXT_ALIGN::Left;
}

/**
 * \brief Parses a string value to BOX_SIZING enum.
 * 
 * \param value CSS box-sizing value (e.g., "border-box", "content-box")
 * \return Corresponding BOX_SIZING enum value
 */
BOX_SIZING COMPUTED_STYLE::parse_box_sizing(const std::string &value)
{
    if (value == "border-box") return BOX_SIZING::BorderBox;
    return BOX_SIZING::ContentBox;
}

/**
 * \brief Parses a string value to TEXT_DECORATION enum.
 * 
 * \param value CSS text-decoration value (e.g., "underline", "line-through", "overline")
 * \return Corresponding TEXT_DECORATION enum value
 */
TEXT_DECORATION COMPUTED_STYLE::parse_text_decoration(const std::string &value)
{
    if (value == "underline") return TEXT_DECORATION::UnderLine;
    if (value == "line-through") return TEXT_DECORATION::LineThrough;
    if (value == "overline") return TEXT_DECORATION::OverLine;
    return TEXT_DECORATION::None;
}

/**
 * \brief Parses a string value to POSITION_TYPE enum.
 * 
 * \param value CSS position value (e.g., "static", "relative", "absolute", "fixed")
 * \return Corresponding POSITION_TYPE enum value
 */
POSITION_TYPE COMPUTED_STYLE::parse_position_type(const std::string &value)
{
    if (value == "relative") return POSITION_TYPE::Relative;
    if (value == "absolute") return POSITION_TYPE::Absolute;
    if (value == "fixed") return POSITION_TYPE::Fixed;
    return POSITION_TYPE::Static;
}

/**
 * \brief Parses CSS spacing shorthand values (margin/padding with 1-4 values).
 * 
 * Implements CSS shorthand syntax:
 * - 1 value: applied to all sides
 * - 2 values: [vertical, horizontal]
 * - 3 values: [top, horizontal, bottom]
 * - 4+ values: [top, right, bottom, left]
 * 
 * \param value CSS spacing shorthand string
 * \return SPACING_VALUES struct with top, right, bottom, left values
 */
COMPUTED_STYLE::SPACING_VALUES COMPUTED_STYLE::parse_spacing_shorthand(const std::string &value)
{
    std::vector<std::string> parts;
    std::stringstream ss(value);
    std::string part;

    while (ss >> part) {
        parts.push_back(part);
    }

    float val_top = 0, val_right = 0, val_bottom = 0, val_left = 0;

    if (parts.size() == 1) {
        // All sides
        float val = parse_string_to_float(parts[0], 0);
        val_top = val_right = val_bottom = val_left = val;
    }
    else if (parts.size() == 2) {
        // Vertical, Horizontal
        float vertical = parse_string_to_float(parts[0], 0);
        float horizontal = parse_string_to_float(parts[1], 0);
        val_top = val_bottom = vertical;
        val_left = val_right = horizontal;
    }
    else if (parts.size() == 3) {
        // Top, Horizontal, Bottom
        val_top = parse_string_to_float(parts[0], 0);
        float horizontal = parse_string_to_float(parts[1], 0);
        val_bottom = parse_string_to_float(parts[2], 0);
        val_left = val_right = horizontal;
    }
    else if (parts.size() >= 4) {
        // Top, Right, Bottom, Left
        val_top = parse_string_to_float(parts[0], 0);
        val_right = parse_string_to_float(parts[1], 0);
        val_bottom = parse_string_to_float(parts[2], 0);
        val_left = parse_string_to_float(parts[3], 0);
    }

    return COMPUTED_STYLE::SPACING_VALUES{val_top, val_right, val_bottom, val_left};
}

// ============================================================================
// Setter Initialization
// ============================================================================
void COMPUTED_STYLE::init_setters()
{
    if (initialized)
        return;
    initialized = true;

    setters.clear();

    setters["color"] = [](COMPUTED_STYLE &style, const std::string &value)
    {
        style.color = parse_color(value);
    };

    setters["font-size"] = [](COMPUTED_STYLE &style, const std::string &value)
    {
        style.font_size = parse_font_size(value);
    };

    setters["font-weight"] = [](COMPUTED_STYLE &style, const std::string &value)
    {
        try
        {
            if (value == "normal")
            {
                style.font_weight = QFont::Normal;
                return;
            }

            if (value == "bold")
            {
                style.font_weight = QFont::Bold;
                return;
            }
            int weight = std::stoi(value);
            int adjusted_weight;
            if (weight >= 100 && weight < 1000)
            {
                adjusted_weight = std::round(weight / 100) * 100;

                if (adjusted_weight <= 100)
                {
                    style.font_weight = QFont::Thin;
                }

                else if (adjusted_weight == 200)
                {
                    style.font_weight = QFont::ExtraLight;
                }

                else if (adjusted_weight == 300)
                {
                    style.font_weight = QFont::Light;
                }

                else if (adjusted_weight == 400)
                {
                    style.font_weight = QFont::Normal;
                }

                else if (adjusted_weight == 500)
                {
                    style.font_weight = QFont::Medium;
                }

                else if (adjusted_weight == 600)
                {
                    style.font_weight = QFont::DemiBold;
                }

                else if (adjusted_weight == 700)
                {
                    style.font_weight = QFont::Bold;
                }

                else if (adjusted_weight == 800)
                {
                    style.font_weight = QFont::ExtraBold;
                }

                else if (adjusted_weight >= 900)
                {
                    style.font_weight = QFont::Black;
                }
            }
            else
            {
                style.font_weight = QFont::Normal;
            }
        }
        catch (...)
        {
            style.font_weight = QFont::Normal;
        }
    };

    setters["font-style"] = [](COMPUTED_STYLE &style, const std::string &value)
    {
        style.font_style = value;
    };

    setters["font-family"] = [](COMPUTED_STYLE &style, const std::string &value)
    {
        style.font_family = QString::fromStdString(value);
    };

    setters["background-color"] = [](COMPUTED_STYLE &style, const std::string &value)
    {
        style.background_color = parse_color(value);
    };

    setters["width"] = [](COMPUTED_STYLE &style, const std::string &value)
    {
        style.width = parse_string_to_float(value, -1);
    };

    setters["height"] = [](COMPUTED_STYLE &style, const std::string &value)
    {
        style.height = parse_string_to_float(value, -1);
    };

    setters["margin-top"] = [](COMPUTED_STYLE &style, const std::string &value)
    {
        style.margin_top = parse_string_to_float(value, 0);
    };

    setters["margin-bottom"] = [](COMPUTED_STYLE &style, const std::string &value)
    {
        style.margin_bottom = parse_string_to_float(value, 0);
    };

    setters["margin-left"] = [](COMPUTED_STYLE &style, const std::string &value)
    {
        style.margin_left = parse_string_to_float(value, 0);
    };

    setters["margin-right"] = [](COMPUTED_STYLE &style, const std::string &value)
    {
        style.margin_right = parse_string_to_float(value, 0);
    };

    setters["margin"] = [](COMPUTED_STYLE &style, const std::string &value)
    {
        auto spacing = COMPUTED_STYLE::parse_spacing_shorthand(value);
        style.margin_top = spacing.top;
        style.margin_right = spacing.right;
        style.margin_bottom = spacing.bottom;
        style.margin_left = spacing.left;
    };

    setters["padding-top"] = [](COMPUTED_STYLE &style, const std::string &value)
    {
        style.padding_top = parse_string_to_float(value, 0);
    };

    setters["padding-bottom"] = [](COMPUTED_STYLE &style, const std::string &value)
    {
        style.padding_bottom = parse_string_to_float(value, 0);
    };

    setters["padding-left"] = [](COMPUTED_STYLE &style, const std::string &value)
    {
        style.padding_left = parse_string_to_float(value, 0);
    };

    setters["padding-right"] = [](COMPUTED_STYLE &style, const std::string &value)
    {
        style.padding_right = parse_string_to_float(value, 0);
    };

    setters["padding"] = [](COMPUTED_STYLE &style, const std::string &value)
    {
        auto spacing = COMPUTED_STYLE::parse_spacing_shorthand(value);
        style.padding_top = spacing.top;
        style.padding_right = spacing.right;
        style.padding_bottom = spacing.bottom;
        style.padding_left = spacing.left;
    };

    setters["border-width"] = [](COMPUTED_STYLE &style, const std::string &value)
    {
        style.border_width = parse_string_to_float(value, 0);
    };

    setters["border-color"] = [](COMPUTED_STYLE &style, const std::string &value)
    {
        style.border_color = parse_color(value);
    };

    setters["border-style"] = [](COMPUTED_STYLE &style, const std::string &value)
    {
        if (value == "solid")
        {
            style.border_style = Qt::SolidLine;
        }

        else if (value == "dashed")
        {
            style.border_style = Qt::DashLine;
        }

        else if (value == "dotted")
        {
            style.border_style = Qt::DotLine;
        }

        else
        {
            style.border_style = Qt::NoPen;
        }
    };

    setters["border"] = [](COMPUTED_STYLE &style, const std::string &value)
    {
        std::vector<std::string> parts;
        std::stringstream ss(value);
        std::string part;

        while (ss >> part)
        {
            parts.push_back(part);
        }

        // Each part could be width, style, or color
        // We need to detect which is which
        for (const auto &p : parts)
        {
            // Check if it's a number (width)
            if (std::isdigit(p[0]))
            {
                style.border_width = parse_string_to_float(p, 0);
            }
            // Check if it's a style keyword
            else if (p == "solid")
            {
                style.border_style = Qt::SolidLine;
            }
            else if (p == "dashed")
            {
                style.border_style = Qt::DashLine;
            }
            else if (p == "dotted")
            {
                style.border_style = Qt::DotLine;
            }
            // Otherwise it's a color
            else
            {
                style.border_color = parse_color(p);
            }
        }
    };

    setters["display"] = [](COMPUTED_STYLE &style, const std::string &value)
    {
        style.display = COMPUTED_STYLE::parse_display_type(value);
    };

    setters["box-sizing"] = [](COMPUTED_STYLE &style, const std::string &value)
    {
        style.box_sizing = COMPUTED_STYLE::parse_box_sizing(value);
    };

    setters["text-align"] = [](COMPUTED_STYLE &style, const std::string &value)
    {
        style.text_align = COMPUTED_STYLE::parse_text_align(value);
    };

    setters["line-height"] = [](COMPUTED_STYLE &style, const std::string &value)
    {
        style.line_height = parse_string_to_float(value, 16 * 1.5);
    };

    setters["visibility"] = [](COMPUTED_STYLE &style, const std::string &value)
    {
        if (value == "hidden" || value == "collapse")
        {
            style.visibility = false;
        }
        else
        {
            style.visibility = true;
        }
    };

    setters["text-decoration"] = [](COMPUTED_STYLE &style, const std::string &value)
    {
        style.text_decoration = COMPUTED_STYLE::parse_text_decoration(value);
    };

    setters["opacity"] = [](COMPUTED_STYLE &style, const std::string &value)
    {
        style.opacity = parse_string_to_float(value, 1);
        if (style.opacity < 0.0)
            style.opacity = 0.0;
        if (style.opacity > 1.0)
            style.opacity = 1.0;
    };

    setters["position"] = [](COMPUTED_STYLE &style, const std::string &value)
    {
        style.position = COMPUTED_STYLE::parse_position_type(value);
    };

    setters["top"] = [](COMPUTED_STYLE &style, const std::string &value)
    {
        style.top = parse_string_to_float(value, 0);
        style.is_top_set = true;
    };

    setters["right"] = [](COMPUTED_STYLE &style, const std::string &value)
    {
        style.right = parse_string_to_float(value, 0);
        style.is_right_set = true;
    };

    setters["bottom"] = [](COMPUTED_STYLE &style, const std::string &value)
    {
        style.bottom = parse_string_to_float(value, 0);
        style.is_bottom_set = true;
    };

    setters["left"] = [](COMPUTED_STYLE &style, const std::string &value)
    {
        style.left = parse_string_to_float(value, 0);
        style.is_left_set = true;
    };
}
/**
 * \brief Parses a CSS color value string into a QColor object.
 *
 * Handles both named colors and RGB/RGBA formats. Named colors are passed directly
 * to QColor for interpretation. RGB format (e.g., "rgb(255,128,0)") is parsed by
 * extracting numeric values and converting to QColor. RGBA format includes an alpha
 * channel (opacity) that is scaled from 0-1 to 0-255.
 *
 * \param color_value The color string to parse (e.g., "red", "rgb(255,0,0)", "rgba(255,0,0,0.5)").
 * \return A QColor object representing the parsed color.
 */QColor COMPUTED_STYLE::parse_color(const std::string &color_value)
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

/**
 * \brief Parses a CSS font-size value string into pixel units.
 *
 * Converts font-size values from various CSS units (px, pt, cm, mm, in) and
 * CSS keywords (xx-small through xx-large) into pixel values. Uses standard
 * conversion factors (1in = 96px = 2.54cm). Returns 16px as the default/medium
 * font size if parsing fails.
 *
 * \param value The font-size value string (e.g., "14px", "medium", "1.5cm").
 * \return The font size in pixels as an integer.
 */
int COMPUTED_STYLE::parse_font_size(const std::string &value)
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

/**
 * \brief Parses a CSS numeric value string with units into a float.
 *
 * Converts string representations of numbers (with or without units) into
 * floating-point values. Handles invalid input gracefully by returning the
 * provided default value.
 *
 * \param value The numeric string to parse (e.g., "10px", "15.5").
 * \param default_value The value to return if parsing fails.
 * \return The parsed float value, or default_value on error.
 */
float COMPUTED_STYLE::parse_string_to_float(const std::string &value, const float default_value)
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

/**
 * \brief Returns the inherited color value as a hex string.
 *
 * Converts the QColor member to its hexadecimal string representation for
 * use in CSS inheritance to child elements.
 *
 * \return The color as a hex string (e.g., "#ff0000").
 */
std::string COMPUTED_STYLE::inherit_color() const
{
    QString color_hex_name = color.name();
    return color_hex_name.toStdString();
}

/**
 * \brief Returns the inherited font-size value as a string.
 *
 * Converts the font_size member (in pixels) to its string representation
 * for inheritance to child elements.
 *
 * \return The font size as a string (e.g., "14").
 */
std::string COMPUTED_STYLE::inherit_font_size() const
{
    return std::to_string(font_size);
}

/**
 * \brief Returns the inherited font-weight value as a string.
 *
 * Converts the font_weight member (QFont::Weight enum) to its numeric
 * CSS representation for inheritance to child elements.
 *
 * \return The font weight as a string (e.g., "700" for bold, "400" for normal).
 */
std::string COMPUTED_STYLE::inherit_font_weight() const
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

/**
 * \brief Returns the inherited font-style value as a string.
 *
 * Returns the font_style member for inheritance to child elements.
 *
 * \return The font style as a string (e.g., "italic", "normal").
 */
std::string COMPUTED_STYLE::inherit_font_style() const
{
    return font_style;
}

/**
 * \brief Returns the inherited font-family value as a string.
 *
 * Converts the font_family member (QString) to std::string for inheritance
 * to child elements.
 *
 * \return The font family as a string.
 */
std::string COMPUTED_STYLE::inherit_font_family() const
{
    return font_family.toStdString();
}

/**
 * \brief Returns the inherited line-height value as a string.
 *
 * Returns the line_height member for inheritance to child elements.
 *
 * \return The line height as a string.
 */
std::string COMPUTED_STYLE::inherit_line_height() const
{
    return std::to_string(line_height);
}

/**
 * \brief Returns the inherited text-align value as a string.
 *
 * Converts the text_align member (TextAlign enum) to its CSS string
 * representation for inheritance to child elements.
 *
 * \return The text alignment as a string (e.g., "left", "center", "right", "justify").
 */
std::string COMPUTED_STYLE::inherit_text_align() const
{
    if (text_align == TEXT_ALIGN::Center)
    {
        return "center";
    }

    else if (text_align == TEXT_ALIGN::Justify)
    {
        return "justify";
    }

    else if (text_align == TEXT_ALIGN::Right)
    {
        return "right";
    }

    else
    {
        return "left";
    }
}

/**
 * \brief Returns the inherited visibility value as a string.
 *
 * Converts the visibility boolean member to its CSS string representation
 * for inheritance to child elements.
 *
 * \return The visibility as a string ("visible" or "hidden").
 */
std::string COMPUTED_STYLE::inherit_visibility() const
{
    if (!visibility)
    {
        return "hidden";
    }
    else
    {
        return "visible";
    }
}

/**
 * \brief Returns the inherited text-decoration value as a string.
 *
 * Converts the text_decoration member (TextDecoration enum) to its CSS string
 * representation for inheritance to child elements.
 *
 * \return The text decoration as a string (e.g., "none", "underline", "line-through", "overline").
 */
std::string COMPUTED_STYLE::inherit_text_decoration() const
{
    switch (text_decoration)
    {
    case TEXT_DECORATION::UnderLine:
        return "underline";
    case TEXT_DECORATION::LineThrough:
        return "line-through";
    case TEXT_DECORATION::OverLine:
        return "overline";
    default:
        return "none";
    }
}
