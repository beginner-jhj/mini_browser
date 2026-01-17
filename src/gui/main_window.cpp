#include "gui/main_window.h"
#include "gui/header.h"
#include "gui/renderer.h"
#include <QVBoxLayout>
#include <QScrollArea>
#include "html/html_tokenizer.h"
#include "html/html_parser.h"

MainWindow::MainWindow(){
    setup_ui();
}


void MainWindow::setup_ui()
{
    setMinimumSize(1000, 800);
    QWidget *centeral_widget = new QWidget(this);
    setCentralWidget(centeral_widget);

    QVBoxLayout *layout = new QVBoxLayout(centeral_widget);

    Header *header = new Header(this);

    layout->addWidget(header,1);

    QScrollArea *rendering_scroll_area = new QScrollArea(this);

    //todo: complete rendering logic. tokenizing, parsing, set_document
    Renderer *renderer = new Renderer(rendering_scroll_area);

    std::shared_ptr<Node> tree = create_tree(m_init_html);
    renderer->set_document(tree);

    rendering_scroll_area->setWidget(renderer);
    rendering_scroll_area->setWidgetResizable(true);

    layout->addWidget(rendering_scroll_area,3);
}

std::shared_ptr<Node> MainWindow::create_tree(const std::string& html){
    auto tokens = tokenize(html);
    auto tree = parse(tokens);
    return tree;
}