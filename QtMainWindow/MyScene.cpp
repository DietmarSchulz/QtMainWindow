#include "MyScene.h"
#include <QGraphicsSceneMouseEvent>

MyScene::MyScene(QObject* parent)
    : QGraphicsScene(parent)
{}

void MyScene::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    QPointF mousePos(event->buttonDownScenePos(Qt::LeftButton).x(),
        event->buttonDownScenePos(Qt::LeftButton).y());
    const QList<QGraphicsItem*> itemList = items(mousePos);
    movingItem = itemList.isEmpty() ? nullptr : itemList.first();

    if (movingItem != nullptr && event->button() == Qt::LeftButton)
        oldPos = movingItem->pos();

    clearSelection();
    QGraphicsScene::mousePressEvent(event);
}

void MyScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    if (movingItem != nullptr && event->button() == Qt::LeftButton) {
        if (oldPos != movingItem->pos())
            emit itemMoved(movingItem,
                oldPos);
        movingItem = nullptr;
    }
    QGraphicsScene::mouseReleaseEvent(event);
}
