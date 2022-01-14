#include <opencv2/opencv.hpp>
#include <filesystem>

#include "qtmainwindow.h"
#include "stdafx.h"
#include "OpenCVWrapper.h"

QtMainWindow::QtMainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    ui.graphicsView->setScene(&scene);
    ui.graphicsView->contextMenu.addAction(ui.action_Rect);
    ui.graphicsView->contextMenu.addAction(ui.action_Picture);

    ui.graphicsView->pictureContextMenu.addAction(ui.action_Brightnesss);
}

void QtMainWindow::on_action_New_triggered()
{
}

void QtMainWindow::on_action_Open_triggered()
{
    return;
}

void QtMainWindow::on_action_Save_triggered()
{
}

void QtMainWindow::on_action_Copy_triggered()
{
}

void QtMainWindow::on_action_Cut_triggered()
{
}

void QtMainWindow::on_action_Paste_triggered()
{
}

void QtMainWindow::on_action_Rect_triggered()
{
    QBrush greenBrush(Qt::green);
    QBrush blueBrush(Qt::blue);
    QPen outlinePen(Qt::black);
    outlinePen.setWidth(2);

    QGraphicsRectItem* rectangle = scene.addRect(100, 0, 80, 100, outlinePen, blueBrush);
    QGraphicsRectItem* rect = scene.addRect(QRectF(0, 0, 100, 100), outlinePen);
    QGraphicsTextItem* text = scene.addText("bogotobogo.com", QFont("Arial", 20));
    // movable text
    text->setFlag(QGraphicsItem::ItemIsMovable);
    rect->setFlag(QGraphicsItem::ItemIsMovable);
    rectangle->setFlag(QGraphicsItem::ItemIsMovable);
}

void QtMainWindow::on_action_Picture_triggered()
{
    QString filename = QFileDialog::getOpenFileName(this, "Hole Bild", currdir, "All picture Files (*.jpg *.jpeg *.png *.tiff *.bmp)");
    if (filename.isEmpty())
        return;
    std::filesystem::path p = filename.toStdString();
    currdir = QString::fromStdString(p.parent_path().string());
    cv::Mat picture = cv::imread(p.generic_string());
    QImage qim = OpenCVWrapper::Mat2QImage(picture);
    auto* qimage = scene.addPixmap(QPixmap::fromImage(qim));
    qimage->setFlag(QGraphicsItem::ItemIsMovable);
    qimage->setFlag(QGraphicsItem::ItemIsSelectable);
    qimage->setScale(0.25);
    return;
}

void QtMainWindow::on_actionZoomIn_triggered()
{
    ui.graphicsView->zoomIn();
}

void QtMainWindow::on_actionZoomOut_triggered()
{
    ui.graphicsView->zoomOut();
}
