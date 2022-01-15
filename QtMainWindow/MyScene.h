#pragma once

#include <QObject>
#include <QGraphicsScene>
#include <qgraphicsitem.h>

class MyScene : public QGraphicsScene
{
    Q_OBJECT

public:
    MyScene(QObject* parent = nullptr);

signals:
    void itemMoved(QGraphicsItem* movedItem, const QPointF& movedFromPosition);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

private:
    QGraphicsItem* movingItem = nullptr;
    QPointF oldPos;
};

