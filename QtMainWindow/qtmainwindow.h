#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_qtmainwindow.h"
#include "MyGraphicsView.h"
#include "MyScene.h"
#include <QUndoStack>
#include <QUndoView>
#include <qcombobox>
#include <qt5/QtPrintSupport/qprinter.h>
#include "OpenCVWrapper.h"

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
    QComboBox myComboBox;
    QClipboard* clipboard;
    bool maybeSave();
    static const int maxrecentfile = 10;
    QAction* recentFiles[maxrecentfile];
    QAction* separatorAct;
public slots:
    void itemMoved(QList<QGraphicsItem*>& movedItems, std::vector<QPointF>& moveStartPositions); 
    void showMessage(QString);        // Show messages on the status bar
    void itemScaled(QGraphicsItem* item, double oldScale);
    void itemRotated(QGraphicsItem* item, double oldRotation);
    void print(QPrinter* printer);
private:
    bool checkSelection(int num);
    OpenCVWrapper lSobel;
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
    void on_action_Textfield_triggered();

    void on_action_ZoomIn_triggered();
    void on_action_ZoomOut_triggered();
 
    void on_action_Delete_triggered();

    void on_action_Brightnesss_triggered();
    void on_action_RGB_scale_triggered();
    void on_action_AddImage_triggered();
    void on_action_Sobel_triggered();
    void on_action_HistogramEqualize_triggered();

    void on_action_Print_triggered();
    void on_action_Print_Preview_triggered();

    void itemMenuAboutToShow();
    void itemMenuAboutToHide();

    void undone(bool checked);
    void redone(bool checked);

    void zoomComboIndexChanged(int index);
    void textChange();
};
