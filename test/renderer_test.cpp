#include <QApplication>
#include <QScrollArea>
#include "rendering/browser_widget.h"
#include "html/html_parser.h"
#include "html/html_tokenizer.h"
#include <iostream>
#include "css/layout_tree.h"
#include <QDebug>

void print_node_styles(std::shared_ptr<Node> node, int depth = 0)
{
    std::string indent(depth * 2, ' ');

    if (node->get_type() == NODE_TYPE::ELEMENT)
    {
        auto style = node->get_all_styles();
        std::cout << indent << "<" << node->get_tag_name() << ">";
        std::cout << " padding: " << style.padding_top << " "
                  << style.padding_right << " "
                  << style.padding_bottom << " "
                  << style.padding_left;
        std::cout << " margin: " << style.margin_top << " "
                  << style.margin_right << " "
                  << style.margin_bottom << " "
                  << style.margin_left;
        std::cout << std::endl;
    }

    for (auto child : node->get_children())
    {
        print_node_styles(child, depth + 1);
    }
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // renderer_test.cpp
   std::string html = R"(
<html>
<head>
    <style>
        body {
            margin: 0;
            padding: 0;
        }
        
        /* Fixed Header */
        .header {
            position: fixed;
            top: 0;
            left: 0;
            height: 60px;
            background-color: #333;
            color: white;
            padding: 20px;
            border-bottom: 3px solid red;
        }
        
        /* Fixed Sidebar */
        .sidebar {
            position: fixed;
            top: 100px;
            right: 0;
            width: 200px;
            height: 300px;
            background-color: lightblue;
            border: 2px solid blue;
            padding: 10px;
        }
        
        /* Main Content */
        .content {
            margin-top: 100px;  /* Header 공간 */
            margin-right: 220px;  /* Sidebar 공간 */
            padding: 20px;
        }
        
        .box {
            background-color: lightgray;
            padding: 20px;
            margin: 20px 0;
            border: 1px solid black;
        }
        
        /* Fixed Button (bottom-right) */
        .fab {
            position: fixed;
            bottom: 20px;
            right: 20px;
            width: 60px;
            height: 60px;
            background-color: red;
            border: 2px solid darkred;
        }
    </style>
</head>
<body>
    <div class="header">
        Fixed Header (Always on top)
    </div>
    
    <div class="sidebar">
        Fixed Sidebar
        (right side)
    </div>
    
    <div class="content">
        <div class="box">Content Box 1</div>
        <div class="box">Content Box 2</div>
        <div class="box">Content Box 3</div>
        <div class="box">Content Box 4</div>
        <div class="box">Content Box 5</div>
        <div class="box">Content Box 6</div>
        <div class="box">Content Box 7</div>
        <div class="box">Content Box 8</div>
        <div class="box">Content Box 9</div>
        <div class="box">Content Box 10</div>
    </div>
    
    <div class="fab">
        FAB
    </div>
</body>
</html>
)";


    auto tokens = tokenize(html);
    auto tree = parse(tokens);

    BrowserWidget *widget = new BrowserWidget();
    widget->set_document(tree);

    // std::cout << "=== Node Styles ===" << std::endl;
    // print_node_styles(tree);
    QScrollArea *scroll = new QScrollArea();
    scroll->setWidget(widget);
    scroll->setWidgetResizable(true);

    scroll->resize(800, 600);
    scroll->show();

    return app.exec();
}