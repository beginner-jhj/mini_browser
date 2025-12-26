#include "node.h"

Node::Node(NODE_TYPE t, const std::string& content):m_type(t){
    if(t == NODE_TYPE::ELEMENT){
        m_tag_name = content;
        m_text = "";
    }else{
        m_text = content;
        m_tag_name = "";
    }
}

void Node::add_child(std::shared_ptr<Node> child){
    m_children.push_back(child);
}

std::string Node::get_tag_name() const{
    return m_tag_name;
}

std::string Node::get_text_content() const{
    return m_text;
}

NODE_TYPE Node::get_type() const {
    return m_type;
}

std::vector<std::shared_ptr<Node>> Node::get_children() const{
    return m_children;
}