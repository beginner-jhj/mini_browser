#include "gui/renderer.h"
#include "css/css_parser.h"
#include "css/apply_style.h"
#include <QScrollArea>
#include <QScrollBar>
#include <QResizeEvent>
#include <queue>
#include "util_functions.h"

/**
 * \brief Constructs a Renderer widget for displaying web content.
 *
 * Initializes the renderer with empty state, ready to receive and display
 * HTML documents. Sets up internal state for layout, history, and rendering.
 *
 * \param parent The parent QWidget for ownership.
 */
Renderer::Renderer(QWidget *parent) : QWidget(parent), m_root(nullptr), m_viewport_height(0), m_viewport_width(0), m_image_cache_manager(nullptr), m_current_history_it(m_history_list.begin())
{
}

float calculate_content_width(const LAYOUT_BOX &box);

/**
 * \brief Sets a new HTML document for rendering and updates history.
 *
 * Stores the document in browser history and triggers rendering with styling
 * and layout calculation. Clears forward history when a new page is loaded.
 *
 * \param root The root Node of the DOM tree to render.
 * \param image_cache_manager Reference to the image cache manager.
 * \param base_url The base URL for resolving relative resources (default is empty).
 */
void Renderer::set_document(std::shared_ptr<NODE> root, IMAGE_CACHE_MANAGER &image_cache_manager, const QString &base_url)
{
    m_image_cache_manager = &image_cache_manager;

    if (m_history_list.size() > 0)
    {
        auto next_it = std::next(m_current_history_it);
        if (next_it != m_history_list.end())
        {
            m_history_list.erase(next_it, m_history_list.end());
        }
    }

    m_history_list.push_back({root, base_url});

    m_current_history_it = std::prev(m_history_list.end());

    render(root, base_url);
}

/**
 * \brief Renders a DOM tree by applying styles, creating layouts, and scheduling paint.
 *
 * Applies user agent and author CSS, creates a layout tree with calculated positions
 * and dimensions, and triggers a repaint. Handles display properties, spacing, and
 * text layout.
 *
 * \param root The root Node of the DOM tree to render.
 * \param base_url The base URL for resolving relative resources.
 */
void Renderer::render(std::shared_ptr<NODE> root, const QString &base_url)
{
    m_root = root;
    m_base_url = base_url;

    if (m_root)
    {
        std::string user_agent_css = R"(
    /* Hidden elements */
    head, style, script, meta, link, title { display: none; }
    
    /* Block elements */
    html, body, div, p, h1, h2, h3, h4, h5, h6, ul, ol, li, footer, header, section { 
        display: block; 
    }   
    
    /* Default spacing for the page */
    body { 
        padding: 8px; 
        margin: 0;
        line-height: 1.2;
    }

    /* Heading Styles */
    h1 { font-size: 32px; font-weight: bold; margin-top: 21px; margin-bottom: 21px; }
    h2 { font-size: 24px; font-weight: bold; margin-top: 19px; margin-bottom: 19px; }
    h3 { font-size: 18px; font-weight: bold; margin-top: 18px; margin-bottom: 18px; }
    h4 { font-size: 16px; font-weight: bold; margin-top: 21px; margin-bottom: 21px; }
    h5 { font-size: 13px; font-weight: bold; margin-top: 22px; margin-bottom: 22px; }
    h6 { font-size: 10px; font-weight: bold; margin-top: 24px; margin-bottom: 24px; }

    /* Paragraph & List Spacing */
    p { margin-top: 16px; margin-bottom: 16px; }
    ul, ol { padding-left: 40px; margin-top: 16px; margin-bottom: 16px; }

    /* Inline elements */
    span, a, strong, em { display: inline; }
    strong { font-weight: bold; }
    em { font-style: italic; }
    a { color: blue; text-decoration: underline; }
)";
        std::string author_css = extract_stylesheets(m_root);

        std::string combined_css = user_agent_css + "\n" + author_css;
        m_cssom = create_cssom(combined_css);

        apply_style(m_root, m_cssom);

        recalculate_layout();
    }
    update();
}

/**
 * \brief Handles window resize events and recalculates layout if needed.
 *
 * Detects width changes and triggers layout recalculation to reflow content.
 * Passes the event to the base class for further processing.
 *
 * \param event The resize event information.
 */
void Renderer::resizeEvent(QResizeEvent *event)
{
    if (m_root && event->oldSize().width() != event->size().width())
    {
        recalculate_layout();
    }

    QWidget::resizeEvent(event);
}

/**
 * \brief Recalculates the layout tree based on current viewport dimensions.
 *
 * Creates a fresh layout tree with the current widget width, calculating all
 * box positions and dimensions. Updates the widget's size hint to accommodate
 * the rendered content.
 */
void Renderer::recalculate_layout()
{
    if (!m_root)
        return;

    int current_width = this->width();
    if (current_width <= 0)
    {
        QWidget *scroll_area = parentWidget(); // Viewport
        if (scroll_area)
            current_width = scroll_area->width();
        if (current_width <= 0)
            current_width = 1000;
    }

    LINE_STATE line(current_width);

    m_layout_tree = create_layout_tree(m_root, current_width, line, m_base_url, m_image_cache_manager);
    m_has_layout = true;

    float content_width = calculate_content_width(m_layout_tree);
    float final_height = m_layout_tree.height;

    float min_width = std::max(static_cast<float>(current_width), content_width);

    this->setMinimumSize(min_width, final_height);
}

/**
 * \brief Paints the rendered layout tree onto the widget surface.
 *
 * Fills the background with white and recursively paints all layout boxes,
 * including text, images, backgrounds, and borders using QPainter.
 *
 * \param event The paint event information (unused, uses full widget rect).
 */
void Renderer::paintEvent(QPaintEvent *event)
{
    if (!m_root)
    {
        return;
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.fillRect(rect(), Qt::white);

    paint_layout(painter, m_layout_tree, 0, 0);
}

// ============================================================================
// Rendering Helper Functions
// ============================================================================

/**
 * \brief Draws the visual aspects of an element box (background, border, image).
 * 
 * Handles rendering the background color, border, and image content of a layout box.
 * Text nodes are not handled by this function.
 * 
 * \param painter The QPainter to draw with.
 * \param box The layout box to draw.
 * \param abs_x The absolute x coordinate in viewport space.
 * \param abs_y The absolute y coordinate in viewport space.
 */
void Renderer::draw_element_box(QPainter &painter, const LAYOUT_BOX &box, float abs_x, float abs_y)
{
    // Draw image if present
    if (!box.image.isNull()) {
        painter.drawPixmap(abs_x, abs_y, box.width, box.height, box.image);
        return;
    }
    
    // Draw background color
    if (box.style.background_color != QColor("transparent")) {
        painter.fillRect(abs_x, abs_y, box.width, box.height, box.style.background_color);
    }

    // Draw border
    if (box.style.border_width > 0) {
        QPen pen;
        pen.setColor(box.style.border_color);
        pen.setStyle(box.style.border_style);
        pen.setWidthF(box.style.border_width);

        painter.setPen(pen);
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(abs_x, abs_y, box.width, box.height);
    }
}

/**
 * \brief Draws text content with alignment, decoration, and special handling.
 * 
 * Renders text nodes with proper alignment (left/center/right), text decoration
 * (underline/strikethrough/overline), and special rendering for list items (bullets).
 * 
 * \param painter The QPainter to draw with.
 * \param box The text layout box to render.
 * \param offset_x The x-offset of the parent in viewport coordinates.
 * \param offset_y The y-offset of the parent in viewport coordinates.
 * \param parent_box The parent element box for alignment calculations.
 */
void Renderer::draw_text_node(QPainter &painter, const LAYOUT_BOX &box, float offset_x, float offset_y, const LAYOUT_BOX *parent_box)
{
    QFont ft = box.style.to_font();
    painter.setFont(ft);
    painter.setPen(box.style.color);

    QFontMetrics metrics(ft);

    // Calculate text alignment offset
    float offset_adjust = 0;
    if (parent_box) {
        float total_width = 0;
        for (const auto &word_box : box.children) {
            total_width += word_box.width;
        }

        if (parent_box->style.text_align == TEXT_ALIGN::Center) {
            offset_adjust = (parent_box->width - total_width) / 2;
        }
        else if (parent_box->style.text_align == TEXT_ALIGN::Right) {
            offset_adjust = parent_box->width - total_width;
        }
    }

    // Draw bullet for list items
    if (parent_box && parent_box->node->get_tag_name() == "li") {
        painter.drawText(offset_x + box.x + offset_adjust, offset_y + box.y + metrics.ascent(), "•");
        offset_adjust += 15;
    }

    // Draw each word with decoration
    for (const auto &word_box : box.children) {
        float word_abs_x = offset_x + word_box.x + offset_adjust;
        float word_abs_y = offset_y + word_box.y;
        float baseline_y = word_abs_y + metrics.ascent();

        // Draw text
        painter.drawText(word_abs_x, baseline_y, QString::fromStdString(word_box.text));

        // Draw text decoration (underline, strikethrough, overline)
        if (word_box.style.text_decoration != TEXT_DECORATION::None) {
            QPen decoration_pen(box.style.color);
            decoration_pen.setWidth(1);
            painter.setPen(decoration_pen);

            float decoration_y = 0;
            switch (box.style.text_decoration) {
            case TEXT_DECORATION::UnderLine:
                decoration_y = baseline_y + 1;
                break;
            case TEXT_DECORATION::LineThrough:
                decoration_y = word_abs_y + metrics.ascent() / 2;
                break;
            default:
                decoration_y = word_abs_y;
                break;
            }

            painter.drawLine(word_abs_x, decoration_y, word_abs_x + word_box.width, decoration_y);
        }
    }

    painter.setPen(box.style.color);
}

// ============================================================================
// Main Paint Layout
// ============================================================================

/**
 * \brief Recursively paints a layout box and its children.
 *
 * Draws background colors, borders, images, text, and text decorations.
 * Handles relative positioning adjustments and opacity changes. Recursively
 * paints child boxes and absolutely/fixed positioned children.
 *
 * \param painter The QPainter to draw with.
 * \param box The layout box to paint.
 * \param offset_x The x-offset of the parent in viewport coordinates.
 * \param offset_y The y-offset of the parent in viewport coordinates.
 * \param parent_box Optional parent box for text alignment calculations.
 */
void Renderer::paint_layout(QPainter &painter, const LAYOUT_BOX &box, float offset_x, float offset_y, const LAYOUT_BOX *parent_box)
{
    float abs_x = offset_x + box.x;
    float abs_y = offset_y + box.y;

    float previous_opacity = painter.opacity();
    painter.setOpacity(previous_opacity * box.style.opacity);

    // Apply relative positioning offset
    if (box.style.position == POSITION_TYPE::Relative) {
        abs_x += box.style.left - box.style.right;
        abs_y += box.style.top - box.style.bottom;
    }

    // Draw element-specific content (background, border, image)
    if (box.node->get_type() == NODE_TYPE::ELEMENT) {
        draw_element_box(painter, box, abs_x, abs_y);
    }

    // Draw text node content (with alignment, decoration, bullets)
    if (box.node->get_type() == NODE_TYPE::TEXT) {
        draw_text_node(painter, box, offset_x, offset_y, parent_box);
        painter.setOpacity(previous_opacity);
        return;
    }

    // Recursively paint children
    for (const auto &child : box.children) {
        paint_layout(painter, child, abs_x, abs_y, &box);
    }

    // Paint positioned children (absolute/fixed)
    for (const auto &abs_child : box.absolute_children) {
        if (abs_child.style.position == POSITION_TYPE::Fixed) {
            paint_fixed(painter, abs_child);
        } else {
            paint_layout(painter, abs_child, abs_x, abs_y, &box);
        }
    }

    painter.setOpacity(previous_opacity);
}

/**
 * \brief Paints a fixed-position layout box adjusted for scroll position.
 *
 * Calculates the screen position of a fixed element, accounting for scroll
 * offset and CSS positioning properties (top, right, bottom, left).
 *
 * \param painter The QPainter to draw with.
 * \param box The fixed-position layout box to paint.
 */
void Renderer::paint_fixed(QPainter &painter, const LAYOUT_BOX &box)
{
    // QScrollArea -> ViewPort(hidden) -> renderer
    QScrollArea *scroll_area = qobject_cast<QScrollArea *>(parentWidget()->parentWidget());
    int scroll_x = 0;
    int scroll_y = 0;

    if (scroll_area)
    {
        scroll_x = scroll_area->horizontalScrollBar()->value();
        scroll_y = scroll_area->verticalScrollBar()->value();
    }

    float draw_x = 0, draw_y = 0;

    if (box.style.is_left_set)
    {
        draw_x = scroll_x + box.style.left;
    }

    else if (box.style.is_right_set)
    {
        draw_x = scroll_x + m_viewport_width - box.width - box.style.right;
    }
    else
    {
        draw_x = scroll_x;
    }

    if (box.style.is_top_set)
    {
        draw_y = scroll_y + box.style.top;
    }

    else if (box.style.is_bottom_set)
    {
        draw_y = scroll_y + m_viewport_height - box.height - box.style.top;
    }
    else
    {
        draw_y = scroll_y;
    }

    float previous_opacity = painter.opacity();
    painter.setOpacity(previous_opacity * box.style.opacity);

    if (box.style.background_color != QColor("transparent"))
    {
        painter.fillRect(draw_x, draw_y, box.width, box.height, box.style.background_color);
    }

    if (box.style.border_width > 0)
    {
        QPen pen;
        pen.setColor(box.style.border_color);
        pen.setStyle(box.style.border_style);
        pen.setWidthF(box.style.border_width);

        painter.setPen(pen);
        painter.setBrush(Qt::NoBrush);

        painter.drawRect(draw_x, draw_y, box.width, box.height);
    }

    for (const auto &child : box.children)
    {
        paint_layout(painter, child, draw_x, draw_y, &box);
    }

    painter.setOpacity(previous_opacity);
}

/**
 * \brief Calculates the total horizontal content width of a layout tree.
 *
 * Traverses the layout tree to find the rightmost extent of all boxes,
 * accounting for their absolute positions. Returns the maximum right edge.
 *
 * \param root The root layout box to measure.
 * \return The total content width in pixels.
 */
float calculate_content_width(const LAYOUT_BOX &root)
{
    float max_right = root.x + root.width;

    std::queue<std::pair<LAYOUT_BOX, float>> q;
    q.push({root, 0.0f});

    while (!q.empty())
    {
        auto [current_box, parent_abs_x] = q.front();
        q.pop();

        float current_abs_x = parent_abs_x + current_box.x;
        float current_right = current_abs_x + current_box.width;

        if (current_right > max_right)
        {
            max_right = current_right;
        }

        for (const auto &child : current_box.children)
        {
            q.push({child, current_abs_x});
        }
    }

    return max_right - root.x;
}

/**
 * \brief Handles mouse press events to detect clicked links.
 *
 * Finds the DOM node at the click position and searches up the tree
 * for an <a> tag with an href attribute. Emits a link_clicked signal
 * with the resolved absolute URL.
 *
 * \param event The mouse event with click position.
 */
void Renderer::mousePressEvent(QMouseEvent *event)
{
    float x = event->pos().x();
    float y = event->pos().y();

    auto clicked_node = find_node_at(x, y);
    if (clicked_node)
    {
        std::string href = bubble_for_link(clicked_node);

        if (!href.empty())
        {
            QString absolute_url = resolve_url(m_base_url, QString::fromStdString(href));
            emit link_clicked(absolute_url);
        }
    }
}

/**
 * \brief Finds the DOM node at a given viewport coordinate.
 *
 * Delegates to find_node_in_box with the root layout box and (0,0) offset.
 *
 * \param x The x-coordinate in viewport space.
 * \param y The y-coordinate in viewport space.
 * \return A shared pointer to the DOM node at the position, or nullptr.
 */
std::shared_ptr<NODE> Renderer::find_node_at(float x, float y)
{
    return find_node_in_box(m_layout_tree, x, y, 0, 0);
}

/**
 * \brief Recursively searches layout boxes to find the node at a coordinate.
 *
 * Performs a depth-first search of layout boxes, checking children first
 * (to handle overlapping elements correctly), then checking if the coordinate
 * is within the current box's bounds.
 *
 * \param box The layout box to search within.
 * \param x The target x-coordinate.
 * \param y The target y-coordinate.
 * \param offset_x The parent's x-offset in viewport space.
 * \param offset_y The parent's y-offset in viewport space.
 * \return A shared pointer to the DOM node at the position, or nullptr.
 */
std::shared_ptr<NODE> Renderer::find_node_in_box(const LAYOUT_BOX &box, float x, float y, float offset_x, float offset_y)
{
    float abs_x = offset_x + box.x;
    float abs_y = offset_y + box.y;

    if (box.style.position == POSITION_TYPE::Relative)
    {
        abs_x += box.style.left - box.style.right;
        abs_y += box.style.top - box.style.bottom;
    }

    // FIRST: Always check children, regardless of this box's bounds
    // Why: For inline elements, parent box might be (0,0) but children have real positions
    for (const auto &child : box.children)
    {
        auto result = find_node_in_box(child, x, y, abs_x, abs_y);
        if (result)
        {
            return result;
        }
    }

    for (const auto &abs_child : box.absolute_children)
    {
        if (abs_child.style.position == POSITION_TYPE::Fixed)
        {
            continue;
        }
        auto result = find_node_in_box(abs_child, x, y, abs_x, abs_y);
        if (result)
        {
            return result;
        }
    }

    // THEN: Check if click is inside this box's bounds
    bool inside = (x >= abs_x && x <= abs_x + box.width &&
                   y >= abs_y && y <= abs_y + box.height);

    if (!inside)
    {
        return nullptr;
    }

    // If no children matched and we're inside, return this node
    if (box.node)
    {
        return box.node;
    }

    return nullptr;
}

/**
 * \brief Searches up the DOM tree for the nearest link anchor element.
 *
 * Starting from a given node, bubbles up the DOM tree looking for an <a> tag
 * with an href attribute. Returns the href value if found, empty string if none.
 *
 * \param node The starting DOM node to search from.
 * \return The href value of the nearest parent <a> tag, or empty string.
 */
std::string Renderer::bubble_for_link(std::shared_ptr<NODE> node)
{
    std::shared_ptr<NODE> current = node;

    while (current)
    {
        if (current->get_tag_name() == "a")
        {
            std::string href = current->get_attribute("href");
            if (!href.empty())
            {
                return href;
            }
        }

        current = current->get_parent();
    }

    return "";
}

/**
 * \brief Navigates to the previous page in browser history.
 *
 * Moves the history iterator backward and re-renders the previous page
 * if available.
 */
void Renderer::go_back()
{
    if (m_current_history_it != m_history_list.begin())
    {
        m_current_history_it = std::prev(m_current_history_it);
        PAGE current_page = *m_current_history_it;
        render(current_page.page_root, current_page.base_url);
    }
}

/**
 * \brief Navigates to the next page in browser history.
 *
 * Moves the history iterator forward and re-renders the next page
 * if available.
 */
void Renderer::go_forward()
{
    auto next_it = std::next(m_current_history_it);
    if (next_it != m_history_list.end())
    {
        m_current_history_it = next_it;
        PAGE current_page = *m_current_history_it;
        render(current_page.page_root, current_page.base_url);
    }
}
