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
        .normal { line-height: 17; }
        .loose { line-height: 30; }
        .tight { line-height: 10; }
    </style>
</head>
<body>
    <p class="normal" style="width:400px;">
        This is normal line height text that wraps. Wow please I hope this work well. I can do anything. I will success!!!! Come on!! No pain no gain!!
    </p>
    <p class="loose">
        This is loose line height text that wraps
    </p>
    <p class="tight">
        This is tight line height text that wraps
    </p>
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