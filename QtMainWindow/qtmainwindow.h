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
#include <memory>

class QtMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    QtMainWindow(QWidget *parent = Q_NULLPTR);
    void PrepareUpdateUI();
    void PreparePictureContextMenu();
    void PrepareGeneralContextMenu();
    void PrepareUndoRedo();
    void PrepareTopologyChanges();
    void PrepareZooming();
    void PicturePropertyConnections();
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
    QComboBox myZoomComboBox;
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
    template<typename T>
        requires std::is_base_of_v<QGraphicsItem, T>
    void fillProps(T* sel);
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
    void on_action_Farbe_triggered();
    void on_action_SubPicture_triggered();

    void on_action_Print_triggered();
    void on_action_Print_Preview_triggered();
    void on_action_Font_triggered();

    void itemMenuAboutToShow();
    void itemMenuAboutToHide();

    void zoomComboIndexChanged(int index);
    void zoomComboStringChanged(const QString& newText);
    void zoomed();
    void fontComboIndexChanged(int index);
    void textChange();
    void setSelectedLabelText(const MyPicture* pic);
    void setSelectedLabelText(const QGraphicsTextItem* txt);
    void setSelectedLabelText(const QGraphicsRectItem* rect);
    void undoIndexChanged(int idx);
    void sceneSelectionChanged();
    void setSelScale(double newScale);
    void setSelRotation(double newScale);
    void setSelGamma(double newGamma);
    void setSelGammaRed(int newGammaRed);
    void setSelGammaGreen(int newGammaGreen);
    void setSelGammaBlue(int newGammaBlue);
    void setSelZvalue(double newZvalue);
    void setSelFont(QString fontName);
};
