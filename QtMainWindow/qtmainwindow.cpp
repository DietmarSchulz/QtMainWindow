#include <opencv2/opencv.hpp>

#include "qtmainwindow.h"
#include "stdafx.h"
#include "OpenCVWrapper.h"

QtMainWindow::QtMainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    ui.graphicsView->setScene(&scene);
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
    cv::Mat pizza = cv::imread(R"(D:\sorted_pics\2021\8\23\20210823_194050.jpg)");
    QImage qim = OpenCVWrapper::Mat2QImage(pizza);
    auto* qpizza = scene.addPixmap(QPixmap::fromImage(qim));
    qpizza->setFlag(QGraphicsItem::ItemIsMovable);
    qpizza->setFlag(QGraphicsItem::ItemIsSelectable);
    qpizza->setScale(0.25);
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
