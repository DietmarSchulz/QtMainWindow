#pragma once
#include <QtWidgets/QMainWindow>
#include <qt5/QtWidgets/qgraphicsview.h>
#include <qt5/QtGui/QWheelEvent>
#include <qt5/QtWidgets/qmenu.h>

#include "MyScene.h"

class MyGraphicsView : public QGraphicsView
{
Q_OBJECT
public:
    QMenu contextMenu;
    QMenu pictureContextMenu;
    MyGraphicsView(QWidget* parent) : QGraphicsView(parent), contextMenu(this), pictureContextMenu(this) {};
public slots:
    void zoomIn() { scale(1.2, 1.2); }
    void zoomOut() { scale(1 / 1.2, 1 / 1.2); }
    void rotateLeft() { rotate(-10); }
    void rotateRight() { rotate(10); }
signals:
    void itemScaled(QGraphicsItem* item, double oldScale);
protected:
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
};

