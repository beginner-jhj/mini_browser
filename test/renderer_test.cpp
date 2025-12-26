#include <iostream>
#include <QApplication>
#include "browser_widget.h"
#include "parser.h"
#include "html_tokenizer.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    std::string html = R"(
<html>
<head>
    <title>My First Browser</title>
</head>
<body>
    <h1>Welcome to MiniBrowser!</h1>
    <p>This is a paragraph with some text.</p>
    <div>
        <p>Nested paragraph inside a div</p>
        <p>Another nested paragraph</p>
    </div>
    <h2>Features</h2>
    <p>Currently supports:</p>
    <p>- HTML parsing</p>
    <p>- DOM tree construction</p>
    <p>- Basic text rendering</p>
    <h2>Coming Soon</h2>
    <p>CSS styling</p>
    <p>Layout engine</p>
    <p>And much more!</p>
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

    for (const auto &token : tokens)
    {
        std::cout << (int)token.type << ": " << token.value << std::endl;
    }

    BrowserWidget widget;
    widget.set_document(tree);
    widget.resize(400, 300);
    widget.show();

    return app.exec();
}