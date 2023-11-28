#include <QtWidgets/qgraphicsitem.h>
#include "MyGraphicsView.h"

void MyGraphicsView::keyPressEvent(QKeyEvent* event)
{
	QGraphicsView::keyPressEvent(event);
}

void MyGraphicsView::keyReleaseEvent(QKeyEvent* event)
{
	QGraphicsView::keyReleaseEvent(event);
}

void MyGraphicsView::mouseDoubleClickEvent(QMouseEvent* event)
{
	QGraphicsView::mouseDoubleClickEvent(event);
}

void MyGraphicsView::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::MouseButton::RightButton) {
        QGraphicsItem* item = scene()->itemAt(mapToScene(event->pos()), QTransform());
        if (item != nullptr && item->type() == QGraphicsPixmapItem::Type) {
            pictureContextMenu.exec(event->globalPos());
        }
        else {
            if (item != nullptr) {
                item->setSelected(true);
            }
            contextMenu.exec(event->globalPos());
        }
    }
	QGraphicsView::mousePressEvent(event);
}

void MyGraphicsView::mouseMoveEvent(QMouseEvent* event)
{
	QGraphicsView::mouseMoveEvent(event);
}

void MyGraphicsView::mouseReleaseEvent(QMouseEvent* event)
{
	QGraphicsView::mouseReleaseEvent(event);
}

void MyGraphicsView::wheelEvent(QWheelEvent* event)
{
    MyScene* myScene = static_cast<MyScene*>(scene());
    if (myScene->selectedItems().empty()) {
        if (event->modifiers() & Qt::ShiftModifier)
        {
            // Rotate
            if (event->angleDelta().y() > 0) {
                rotateLeft();
            }
            else {
                rotateRight();
            }
        }
        else if (event->modifiers() & Qt::ControlModifier)
        {
            // Zoom
            if (event->angleDelta().y() > 0) {
                zoomIn();
            }
            else {
                zoomOut();
            }
        }
        else
        {
            QGraphicsView::wheelEvent(event);
        }
    }
    else {
        auto* sceneItem = myScene->selectedItems().first();
        myScene->SetModified(true);
        if (event->modifiers() & Qt::ShiftModifier)
        {
            // Rotate
            auto currRot = sceneItem->rotation();
            if (event->angleDelta().y() > 0) {
                sceneItem->setRotation(currRot - 10);
            }
            else {
                sceneItem->setRotation(currRot + 10);
            }
            emit itemRotated(sceneItem, currRot);
        }
        else if (event->modifiers() & Qt::ControlModifier)
        {
            // Zoom
            auto currScale = sceneItem->scale();
            if (event->angleDelta().y() > 0) {
                sceneItem->setScale(currScale * 1.2);
            }
            else {
                sceneItem->setScale(currScale / 1.2);
            }
            emit itemScaled(sceneItem, currScale);
        }
        else
        {
            QGraphicsView::wheelEvent(event);
        }
    }
}
