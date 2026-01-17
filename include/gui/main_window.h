#pragma once
#include <QMainWindow>
#include "html/node.h"

class MainWindow: public QMainWindow{
    Q_OBJECT

    private:
        void setup_ui();

std::string m_init_html = R"(
    <!DOCTYPE html>
    <html>
        <head>
        </head>
        <body>
            <h1>🌐 Welcome to My Custom Browser</h1>
            
            <p>A lightweight browser engine built from scratch with C++ and Qt.</p>
            
            <h2>Current Features</h2>
            <ul>
                <li>HTML tokenization and parsing</li>
                <li>CSS tokenization and parsing</li>
                <li>Basic styling and layout engine</li>
                <li>Rendering pipeline implementation</li>
            </ul>
            
            <h2>Design Philosophy</h2>
            <p>Built with a focus on understanding browser internals, 
               this project demonstrates core web rendering concepts 
               without relying on existing browser engines.</p>
            
            <footer>
                <p><em>Developed by 현진</em></p>
            </footer>
        </body>
    </html>
)";

    public:
       explicit MainWindow();
       std::shared_ptr<Node> create_tree(const std::string &html);
};