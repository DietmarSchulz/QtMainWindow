#include <opencv2/opencv.hpp>
#include <filesystem>

#include "qtmainwindow.h"
#include "stdafx.h"
#include "OpenCVWrapper.h"
#include "Commands.h"
#include "MyPicture.h"

QtMainWindow::QtMainWindow(QWidget *parent)
    : QMainWindow(parent), undoStack(this), undoView(&undoStack), scene(this)
{
    ui.setupUi(this);
    ui.graphicsView->setScene(&scene);
    ui.graphicsView->contextMenu.addAction(ui.action_Rect);
    ui.graphicsView->contextMenu.addAction(ui.action_Picture);

    ui.graphicsView->pictureContextMenu.addAction(ui.action_Brightnesss);
    undoView.setWindowTitle(tr("Kommando Liste"));
    undoView.show();
    undoView.setAttribute(Qt::WA_QuitOnClose, false);

    connect(&scene, &MyScene::itemMoved,
        this, &QtMainWindow::itemMoved);

    connect(ui.menuEdit, &QMenu::aboutToShow,
        this, &QtMainWindow::itemMenuAboutToShow);
    connect(ui.menuEdit, &QMenu::aboutToHide,
        this, &QtMainWindow::itemMenuAboutToHide);
}

void QtMainWindow::on_action_New_triggered()
{
    if (maybeSave()) {
        scene.New();
        undoStack.clear();
    }
}

void QtMainWindow::itemMoved(QGraphicsItem* movedItem, const QPointF& moveStartPosition)
{
    undoStack.push(new MoveCommand(movedItem, moveStartPosition));
}

void QtMainWindow::closeEvent(QCloseEvent* event)
{
    if (maybeSave()) {
        event->accept();
    }
    else {
        event->ignore();
    }
}

bool QtMainWindow::maybeSave()
{
    if (!scene.isModified()) {
        return true;
    }
    const QMessageBox::StandardButton ret
        = QMessageBox::warning(this, "Applikation",
            "Die Szene wurde modifiziert.\n"
                "Sollen die Anpassungen gespeichert werden?",
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    switch (ret) {
    case QMessageBox::Save:
        on_action_Save_triggered();
        return false;
    case QMessageBox::Cancel:
        return false;
    default:
        break;
    }
    return true;
}

void QtMainWindow::on_action_Open_triggered()
{
    if (maybeSave()) {
        scene.New();
        undoStack.clear();
        QString filename = QFileDialog::getOpenFileName(this, "Lade Szene", currdir, "Json File (*.json)");
        if (filename.isEmpty())
            return;
        std::filesystem::path p = filename.toStdString();
        currdir = QString::fromStdString(p.parent_path().string());
        scene.load(filename);
    }
}

void QtMainWindow::on_action_Save_triggered()
{
    if (!scene.save()) {
        on_action_SaveAs_triggered();
    }
}

void QtMainWindow::on_action_SaveAs_triggered()
{
    QString filename = QFileDialog::getSaveFileName(this, "Hole Bild", currdir, "Json File (*.json)");
    if (filename.isEmpty())
        return;
    std::filesystem::path p = filename.toStdString();
    currdir = QString::fromStdString(p.parent_path().string());
    scene.save(filename);
}

void QtMainWindow::on_action_Copy_triggered()
{
}

void QtMainWindow::on_action_Cut_triggered()
{
    scene.SetModified(true);
}

void QtMainWindow::on_action_Paste_triggered()
{
    scene.SetModified(true);
}

void QtMainWindow::on_action_Rect_triggered()
{
    QBrush greenBrush(Qt::green);
    QBrush blueBrush(Qt::blue);
    QPen outlinePen(Qt::black);
    outlinePen.setWidth(2);

    QGraphicsRectItem* rectangle = new QGraphicsRectItem(100, 0, 80, 100);
    rectangle->setBrush(greenBrush);
    rectangle->setPen(outlinePen);
    rectangle->setFlag(QGraphicsItem::ItemIsMovable);
    rectangle->setFlag(QGraphicsItem::ItemIsSelectable);
    undoStack.push(new AddBoxCommand(rectangle, &scene));

    QGraphicsRectItem* rect = new QGraphicsRectItem(0, 0, 100, 100);
    rect->setBrush(blueBrush);
    rect->setPen(outlinePen);
    rect->setFlag(QGraphicsItem::ItemIsMovable);
    rect->setFlag(QGraphicsItem::ItemIsSelectable);
    undoStack.push(new AddBoxCommand(rect, &scene));

    QGraphicsTextItem* text = new QGraphicsTextItem("bogotobogo.com");
    text->setFont(QFont("Arial", 20));
    text->setFlag(QGraphicsItem::ItemIsMovable);
    text->setFlag(QGraphicsItem::ItemIsSelectable);
    undoStack.push(new AddBoxCommand(text, &scene));
    scene.SetModified(true);
}

void QtMainWindow::on_action_Picture_triggered()
{
    QString filename = QFileDialog::getOpenFileName(this, "Hole Bild", currdir, "All picture Files (*.jpg *.jpeg *.png *.tiff *.bmp)");
    if (filename.isEmpty())
        return;
    std::filesystem::path p = filename.toStdString();
    currdir = QString::fromStdString(p.parent_path().string());
    undoStack.push(new AddPictureCommand(p.generic_string(), &scene));
    scene.SetModified(true);
}

void QtMainWindow::on_action_ZoomIn_triggered()
{
    ui.graphicsView->zoomIn();
}

void QtMainWindow::on_action_ZoomOut_triggered()
{
    ui.graphicsView->zoomOut();
}

void QtMainWindow::on_action_Undo_triggered()
{
    undoStack.undo();
}

void QtMainWindow::on_action_Redo_triggered()
{
    undoStack.redo();
}

void QtMainWindow::on_action_Delete_triggered()
{
    if (scene.selectedItems().isEmpty())
        return;

    QUndoCommand* deleteCommand = new DeleteCommand(&scene);
    undoStack.push(deleteCommand);
}

void QtMainWindow::itemMenuAboutToShow()
{
    ui.action_Delete->setEnabled(!scene.selectedItems().isEmpty());
}

void QtMainWindow::itemMenuAboutToHide()
{
    ui.action_Delete->setEnabled(true);
}
