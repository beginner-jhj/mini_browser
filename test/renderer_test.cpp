#include <QApplication>
#include <QScrollArea>
#include "rendering/browser_widget.h"
#include "html/html_parser.h"
#include "html/html_tokenizer.h"
#include <iostream>
#include "css/layout_tree.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // renderer_test.cpp
   std::string html = R"(
<html>
<head>
    <style>
        .box1 { 
            padding-top: 20;
            padding-bottom: 20;
            padding-left: 30;
            padding-right: 30;
            border-width: 2;
            border-color: red;
            background-color: #ffeeee;
        }
        .box2 { 
            margin-top: 10;
            padding-top: 10;
            padding-left: 50;
            border-width: 2;
            border-color: blue;
            background-color: #eeeeff;
        }
    </style>
</head>
<body>
    <p class="box1">Paragraph with padding - text should be away from border</p>
    <p class="box2">Paragraph with both margin and padding</p>
    <p>Normal paragraph</p>
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