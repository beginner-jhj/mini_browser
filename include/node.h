#pragma once
#include <string>
#include <vector>
#include <memory>
#include <map>

enum class NODE_TYPE{
    ELEMENT,TEXT
};

enum class DISPLAY_TYPE{
    BLOCK, INLINE
};

class Node{
    private:
        NODE_TYPE m_type;
        std::string m_tag_name;
        std::string m_text;

        std::vector<std::shared_ptr<Node>> m_children;
        std::map<std::string, std::string> m_attributes;


    public:
        Node(NODE_TYPE t, const std::string& content);

        void add_child(std::shared_ptr<Node> child);
        void set_attribute(const std::string& name, const std::string& value);
        std::string get_attribute(const std::string& name) const;

        const std::string get_tag_name() const;
        const std::string get_text_content() const;
        const NODE_TYPE get_type() const;
        const DISPLAY_TYPE get_display_type() const;

        const std::vector<std::shared_ptr<Node>>& get_children() const;

};
