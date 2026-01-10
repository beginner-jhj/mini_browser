#include <QApplication>
#include <QScrollArea>
#include "rendering/browser_widget.h"
#include "html/html_parser.h"
#include "html/html_tokenizer.h"
#include <iostream>
#include "css/layout_tree.h"
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // renderer_test.cpp
   std::string html = R"(
<html>
<head>
    <style>
        .box1 { 
            background-color: lightblue; 
            padding: 20px;
        }
        
        .box2 { 
            background-color: lightgreen; 
            padding: 10px 30px;
            margin: 20px;
        }
        
        .box3 { 
            background-color: lightcoral; 
            padding: 5px 15px 25px;
            margin: 10px 20px;
        }
        
        .box4 { 
            background-color: lightyellow; 
            padding: 5px 10px 15px 20px;
            margin: 5px 10px 15px 20px;
        }
    </style>
</head>
<body>
    <div class="box1">Box 1: padding: 20px</div>
    <div class="box2">Box 2: padding: 10px 30px, margin: 20px</div>
    <div class="box3">Box 3: padding: 5px 15px 25px, margin: 10px 20px</div>
    <div class="box4">Box 4: padding: 5px 10px 15px 20px</div>
</body>
</html>
)";

    auto tokens = tokenize(html);
    auto tree = parse(tokens);

    BrowserWidget *widget = new BrowserWidget();
    widget->set_document(tree);
    QScrollArea *scroll = new QScrollArea();
    scroll->setWidget(widget);
    scroll->setWidgetResizable(true);

    scroll->resize(800, 600);
    scroll->show();

    return app.exec();
}