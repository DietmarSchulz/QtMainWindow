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
    bool eventFilter(QObject* watched, QEvent* event) override;

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    void UpdateRecentFileActions();
    void setCurrentFile(const QString& fileName);

    Ui::QtMainWindowClass ui;
    MyScene scene;
    QString currdir{ "D:/sorted_pics" };

    QUndoStack undoStack;
    QUndoView undoView;
    QClipboard* clipboard;
    bool maybeSave();
    static const int maxrecentfile = 10;
    QAction* recentFiles[maxrecentfile];
    QAction* separatorAct;
public slots:
    void itemMoved(QGraphicsItem* movedDiagram, const QPointF& moveStartPosition); 
    void showMessage(QString);        // Show messages on the status bar
private slots:
    void OpenRecentFile();
    void on_action_New_triggered();
    void on_action_Open_triggered();
    void on_action_Save_triggered();
    void on_action_SaveAs_triggered();

    void on_action_Copy_triggered();
    void on_action_Cut_triggered();
    void on_action_Paste_triggered();

    void on_action_Rect_triggered();
    void on_action_Picture_triggered();

    void on_action_ZoomIn_triggered();
    void on_action_ZoomOut_triggered();
 
    void on_action_Delete_triggered();

    void on_action_Brightnesss_triggered();

    void itemMenuAboutToShow();
    void itemMenuAboutToHide();

    void undone(bool checked);
    void redone(bool checked);

    void textChange();
};
