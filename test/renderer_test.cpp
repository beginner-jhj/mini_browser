#include <iostream>
#include <QApplication>
#include <QScrollArea>
#include "rendering/browser_widget.h"
#include "html/html_parser.h"
#include "html/html_tokenizer.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    std::string html = R"(
<html>
<body>
    <h1 style="color: blue; font-size: 48px;">Blue 48px Heading</h1>
    
    <p style="color: red; font-size: 20px;">Red 20px paragraph</p>
    
    <p style="font-size: 16px;">Normal 16px with 
       <strong style="color: green; font-size: 24px;">green 24px bold</strong> text.
    </p>
    
    <h2 style="color: #ff00ff; font-size: 2cm;">Purple 2cm H2</h2>
    
    <p style="color: rgba(117, 33, 77, 0.58); font-size: 15pt;">
        Transparent text using RGBA, 15pt
    </p>
    
    <ul style="font-size: 18px;">
        <li style="color: darkblue; font-size: xx-small;">xx-small Dark Blue</li>
        <li style="color: crimson; font-size: x-small;">x-small Crimson</li>
        <li style="font-size: small;">small Normal</li>
        <li style="font-size: medium;">medium Normal</li>
        <li style="font-size: large;">large Normal</li>
        <li style="font-size: x-large;">x-large Normal</li>
        <li style="font-size: xx-large;">xx-large Normal</li>
    </ul>
    
    <p style="font-size: 10mm;">
        Visit <a href="#" style="color: cyan; font-size: 0.5in;">
            cyan 0.5 inch link
        </a> for more. (10mm text)
    </p>
    
    <p style="font-size: 8px;">Tiny 8px</p>
    <p style="font-size: 12px;">Small 12px</p>
    <p style="font-size: 16px;">Normal 16px</p>
    <p style="font-size: 24px;">Big 24px</p>
    <p style="font-size: 32px;">Huge 32px</p>
    <p style="font-size: 64px;">Giant 64px</p>
</body>
</html>
)";
    auto tokens = tokenize(html);
    auto tree = parse(tokens);

    if (!tree)
    {
        std::cout << "Tree is null!" << std::endl;
    }
    else
    {
        std::cout << "Root: " << tree->get_tag_name() << std::endl;
        std::cout << "Children: " << tree->get_children().size() << std::endl;
    }

    BrowserWidget *widget = new BrowserWidget();
    widget->set_document(tree);

    QScrollArea *scroll = new QScrollArea();
    scroll->setWidget(widget);
    scroll->setWidgetResizable(true);

    widget->setMinimumHeight(2000);

    scroll->resize(600, 400);
    scroll->show();

    return app.exec();
}