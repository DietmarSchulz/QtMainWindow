#include "MyScene.h"
#include <QGraphicsSceneMouseEvent>
#include <QJsonArray>
#include <QtCore/qfile.h>
#include <qjsondocument.h>

MyScene::MyScene(QObject* parent)
    : QGraphicsScene(parent)
{
    setSceneRect(0, 0, 200, 200);
}

void MyScene::load(QString& loadPath)
{
}

bool MyScene::save()
{
    if (filePath.isEmpty()) {
        return false;
    }
    return save(filePath);
}

bool MyScene::save(QString& savePath)
{
    filePath = savePath;
    QFile saveFile(savePath);
    if (!saveFile.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open save file.");
        return false;
    }
    QJsonObject sceneObject;
    write(sceneObject);
    saveFile.write(QJsonDocument(sceneObject).toJson());
    return true;
}

void MyScene::read(const QJsonObject& json)
{
}

void MyScene::write(QJsonObject& json) const
{
    QJsonArray jPictures;

    for (const auto* it : items()) {
        QJsonObject jItem;
        switch (it->type()) {
            case QGraphicsPixmapItem::Type:
                (static_cast<const MyPicture*>(it))->write(jItem);
                jPictures.append(jItem);
                break;
        }
    }
    json["Pictures"] = jPictures;
}

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
