#include "html/node.h"
#include <QDebug>

Node::Node(NODE_TYPE t, const std::string &content) : m_type(t)
{
    if (t == NODE_TYPE::ELEMENT)
    {
        m_tag_name = content;
        m_text = "";
    }
    else
    {
        m_text = content;
        m_tag_name = "";
    }
}

void Node::add_child(std::shared_ptr<Node> child)
{
    m_children.push_back(child);
}

const std::string Node::get_tag_name() const
{
    return m_tag_name;
}

const std::string Node::get_text_content() const
{
    return m_text;
}

const NODE_TYPE Node::get_type() const
{
    return m_type;
}

const DISPLAY_TYPE Node::get_display_type() const
{
    if (m_type == NODE_TYPE::TEXT)
    {
        return DISPLAY_TYPE::INLINE;
    }

    if (m_tag_name == "div" ||
        m_tag_name == "p" ||
        m_tag_name == "h1" ||
        m_tag_name == "h2" ||
        m_tag_name == "ul" ||
        m_tag_name == "li" ||
        m_tag_name == "ul" ||
        m_tag_name == "ol" ||
        m_tag_name == "li")
    {
        return DISPLAY_TYPE::BLOCK;
    }

    return DISPLAY_TYPE::INLINE;
}

const std::vector<std::shared_ptr<Node>> &Node::get_children() const
{
    return m_children;
}

void Node::set_attribute(const std::string &name, const std::string &value)
{
    if (!name.empty() && !value.empty())
    {
        m_attributes[name] = value;
    }
}

std::string Node::get_attribute(const std::string &name) const
{
    try
    {
        return m_attributes.at(name);
    }
    catch (...)
    {
        return "";
    }
}

void Node::set_style(const std::string &name, const std::string &value)
{
    if (m_computed_style.setters.count(name))
    {
        m_computed_style.setters[name](m_computed_style, value);
    }
}

// std::string Node::get_style(const std::string &property) const
// {
// }

ComputedStyle Node::get_all_styles() const
{
    return m_computed_style;
}

void Node::set_rect(float x, float y, float width, float height)
{
    m_rect.setX(x);
    m_rect.setY(y);
    m_rect.setWidth(width);
    m_rect.setHeight(height);
}
