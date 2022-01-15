#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_qtmainwindow.h"
#include "MyGraphicsView.h"
#include "MyScene.h"
#include <QUndoStack>
#include <QUndoView>

class QtMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    QtMainWindow(QWidget *parent = Q_NULLPTR);

private:
    Ui::QtMainWindowClass ui;
    MyScene scene;
    QString currdir{ "D:/sorted_pics" };

    QUndoStack undoStack;
    QUndoView undoView;
public slots:
    void itemMoved(QGraphicsItem* movedDiagram, const QPointF& moveStartPosition); 
private slots:
    void on_action_New_triggered();
    void on_action_Open_triggered();
    void on_action_Save_triggered();

    void on_action_Copy_triggered();
    void on_action_Cut_triggered();
    void on_action_Paste_triggered();

    void on_action_Rect_triggered();
    void on_action_Picture_triggered();

    void on_action_ZoomIn_triggered();
    void on_action_ZoomOut_triggered();
 
    void on_action_Undo_triggered();
    void on_action_Redo_triggered();
};
