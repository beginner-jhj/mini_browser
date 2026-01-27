#pragma once
#include <QColor>
#include <QFont>
#include <QFontMetrics>
#include <functional>
#include <unordered_map>

enum class BOX_SIZING
{
    ContentBox,
    BorderBox
};
enum class TEXT_ALIGN
{
    Left,
    Center,
    Right,
    Justify
};

enum class DISPLAY_TYPE
{
    BLOCK,
    INLINE,
    NONE
};

enum class TEXT_DECORATION
{
    None,
    UnderLine,
    LineThrough,
    OverLine
};

enum class POSITION_TYPE
{
    Static,
    Relative,
    Absolute,
    Fixed
};

struct COMPUTED_STYLE
{
    QColor color = QColor("#000000");          // default: black
    int font_size = 16;                        // default: 16px
    QFont::Weight font_weight = QFont::Normal; // default: normal
    std::string font_style = "normal";
    QString font_family = "Arial";

    QColor background_color = QColor("transparent");
    float width = -1.0;  // -1 = auto
    float height = -1.0; // -1 = auto

    float margin_top = 0.0;
    float margin_right = 0.0;
    float margin_bottom = 0.0;
    float margin_left = 0.0;
    std::string margin{};

    float padding_top = 0.0;
    float padding_right = 0.0;
    float padding_bottom = 0.0;
    float padding_left = 0.0;
    std::string padding{};

    float border_width = 0.0;
    QColor border_color = QColor("#000000");
    Qt::PenStyle border_style = Qt::SolidLine;
    std::string border{};

    DISPLAY_TYPE display = DISPLAY_TYPE::INLINE;
    BOX_SIZING box_sizing = BOX_SIZING::ContentBox;
    TEXT_ALIGN text_align = TEXT_ALIGN::Left;
    TEXT_DECORATION text_decoration = TEXT_DECORATION::None;

    float line_height = font_size * 1.5;
    float opacity = 1;

    POSITION_TYPE position = POSITION_TYPE::Static;
    float top = 0.0;
    bool is_top_set = false;
    float right = 0.0;
    bool is_right_set = false;
    float bottom = 0.0;
    bool is_bottom_set = false;
    float left = 0.0;
    bool is_left_set = false;

    bool visibility = true;

    QFont to_font() const
    {
        QFont font;
        font.setPixelSize(font_size);
        font.setWeight(font_weight);
        font.setFamily(font_family);
        if (font_style == "italic")
        {
            font.setItalic(true);
        }
        return font;
    }
    QFontMetrics font_metrics() const
    {
        return QFontMetrics(to_font());
    }

    using Setter = std::function<void(COMPUTED_STYLE &, const std::string &)>;
    static std::unordered_map<std::string, Setter> setters;

    static QColor parse_color(const std::string &color_value);
    static int parse_font_size(const std::string &value);
    static float parse_string_to_float(const std::string &value, const float default_value = 0);
    
    // Enum parsers
    static DISPLAY_TYPE parse_display_type(const std::string &value);
    static TEXT_ALIGN parse_text_align(const std::string &value);
    static BOX_SIZING parse_box_sizing(const std::string &value);
    static TEXT_DECORATION parse_text_decoration(const std::string &value);
    static POSITION_TYPE parse_position_type(const std::string &value);
    
    // Spacing shorthand parser (margin/padding: 1-4 values)
    struct SPACING_VALUES {
        float top, right, bottom, left;
    };
    static SPACING_VALUES parse_spacing_shorthand(const std::string &value);

    static void init_setters();

    std::string inherit_color() const;
    std::string inherit_font_size() const;
    std::string inherit_font_weight() const;
    std::string inherit_font_style() const;
    std::string inherit_font_family() const;
    std::string inherit_line_height() const;
    std::string inherit_text_align() const;
    std::string inherit_visibility() const;
    std::string inherit_text_decoration() const;
};