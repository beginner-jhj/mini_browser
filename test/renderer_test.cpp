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
<body>
    <h1>My Page</h1>
    <p>This is a paragraph with <strong>bold</strong> and <em>italic</em> text.</p>
    
    <h2>Shopping List</h2>
    <ul>
        <li>Milk</li>
        <li>Eggs</li>
        <li>Bread</li>
    </ul>
    
    <h2>Instructions</h2>
    <ol>
        <li>First step</li>
        <li>Second step</li>
        <li>Third step</li>
    </ol>
    
    <p>Visit <a href="https://example.com">this link</a> for more.</p>
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