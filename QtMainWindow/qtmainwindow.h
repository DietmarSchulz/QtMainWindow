#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_qtmainwindow.h"
#include "MyGraphicsView.h"

class QtMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    QtMainWindow(QWidget *parent = Q_NULLPTR);

private:
    Ui::QtMainWindowClass ui;
    QGraphicsScene scene;
private slots:
    void on_action_New_triggered();
    void on_action_Open_triggered();
    void on_action_Save_triggered();

    void on_action_Copy_triggered();
    void on_action_Cut_triggered();
    void on_action_Paste_triggered();

    void on_action_Rect_triggered();
    void on_action_Picture_triggered();
};
