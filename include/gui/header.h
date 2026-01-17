#pragma once
#include <QWidget>
#include <QPushButton>
#include <QComboBox>

class Header: public QWidget{
    Q_OBJECT


    private:
        QPushButton *m_file_open_button;
        QComboBox *m_url_dropdown;
        QPushButton *m_go_button;
        QString m_selected_url;

        void draw();
        void set_event_conn();

    signals:
        void url_selected(const QString& url);
    public:
        Header(QWidget *parent=nullptr);
};