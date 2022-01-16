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
    QJsonArray jTexts;
    QJsonArray jRects;

    for (const auto* it : items()) {
        QJsonObject jItem;
        switch (it->type()) {
            case QGraphicsPixmapItem::Type:
                (static_cast<const MyPicture*>(it))->write(jItem);
                jPictures.append(jItem);
                break;
            case QGraphicsTextItem::Type:
                write(static_cast<const QGraphicsTextItem*>(it), jItem);
                jTexts.append(jItem);
                break;
            case QGraphicsRectItem::Type:
                write(static_cast<const QGraphicsRectItem*>(it), jItem);
                jRects.append(jItem);
                break;
        }
    }
    json["Pictures"] = jPictures;
    json["Texts"] = jTexts;
    json["Rects"] = jRects;
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

void MyScene::write(const QGraphicsRectItem* rectItem, QJsonObject& jObject) const
{
    jObject["positionX"] = rectItem->pos().x();
    jObject["positionY"] = rectItem->pos().y();
    jObject["width"] = rectItem->rect().width();
    jObject["height"] = rectItem->rect().height();
    jObject["penColor"] = (int) rectItem->pen().color().rgba();
    jObject["penWidth"] = rectItem->pen().width();
    jObject["brushColor"] = (int) rectItem->brush().color().rgba();
    jObject["scale"] = rectItem->scale();
}

void MyScene::write(const QGraphicsTextItem* textItem, QJsonObject& jObject) const
{
    jObject["text"] = textItem->toPlainText();
    jObject["positionX"] = textItem->pos().x();
    jObject["positionY"] = textItem->pos().y();
    jObject["scale"] = textItem->scale();
}

void MyScene::read(QGraphicsRectItem* rectItem, const QJsonObject& jObject)
{
}

void MyScene::read(QGraphicsTextItem* rectItem, const QJsonObject& jObject)
{
}
