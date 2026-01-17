#include "gui/header.h"
#include <QHBoxLayout>
#include <QFileDialog>

Header::Header(QWidget *parent) : QWidget(parent), m_file_open_button(nullptr), m_url_dropdown(nullptr), m_go_button(nullptr)
{
    draw();
    set_event_conn();
}

void Header::draw()
{
    QHBoxLayout *header_box = new QHBoxLayout(this);

    m_file_open_button = new QPushButton("File");
    header_box->addWidget(m_file_open_button, 2);

    m_url_dropdown = new QComboBox(this);
    m_url_dropdown->setPlaceholderText("Select an URL");
    m_url_dropdown->addItem("http://info.cern.ch/");
    m_url_dropdown->addItem("https://www.spacejam.com/1996/");
    m_url_dropdown->addItem("http://motherfuckingwebsite.com/");
    m_url_dropdown->addItem("http://bettermotherfuckingwebsite.com/");
    header_box->addWidget(m_url_dropdown, 3);

    m_go_button = new QPushButton("Go");
    header_box->addWidget(m_go_button,1);
}

void Header::set_event_conn(){
    connect(m_file_open_button, &QPushButton::clicked, this, [this](){
    QString file_name = QFileDialog::getOpenFileName(this,"Select a html file.","","Web Files (*.html *.css);;All Files (*.*)");
    });

    connect(m_url_dropdown, &QComboBox::currentTextChanged,this, [this](const QString& url){
        if(!url.isEmpty()){
            this->m_selected_url = url;
        }
    });

    connect(m_go_button, &QPushButton::clicked, this, [this](){
        emit url_selected(m_selected_url);
    });
}