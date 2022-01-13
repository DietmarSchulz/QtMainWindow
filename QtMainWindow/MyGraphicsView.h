#pragma once
#include <QtWidgets/QMainWindow>
#include <qt5/QtWidgets/qgraphicsview.h>

class MyGraphicsView : public QGraphicsView
{
public:
    MyGraphicsView(QWidget* parent) : QGraphicsView(parent) {};
protected:
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
};

