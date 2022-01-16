#pragma once

#include <QObject>
#include <QGraphicsScene>
#include "MyPicture.h"

class MyScene : public QGraphicsScene
{
    Q_OBJECT

public:
    MyScene(QObject* parent = nullptr);
    void load(QString& loadPath);
    bool save();
    bool save(QString& savePath);

    void read(const QJsonObject& json);
    void write(QJsonObject& json) const;
signals:
    void itemMoved(QGraphicsItem* movedItem, const QPointF& movedFromPosition);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

private:
    QGraphicsItem* movingItem = nullptr;
    QPointF oldPos;
    QString filePath;
};

