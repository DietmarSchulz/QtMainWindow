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
    QFile loadFile(loadPath);
    if (!loadFile.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open save file.");
        return;
    }
    QByteArray saveData = loadFile.readAll();

    QJsonDocument loadDoc(QJsonDocument::fromJson(saveData));

    read(loadDoc.object());
    filePath = loadPath;
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
    if (json.contains("Pictures") && json["Pictures"].isArray()) {
        QJsonArray pictureArray = json["Pictures"].toArray();
        for (auto jObjRef : pictureArray) {
            auto* pPicture = new MyPicture();
            pPicture->read(jObjRef.toObject());
            addItem(pPicture);
        }
    }
    if (json.contains("Texts") && json["Texts"].isArray()) {
        QJsonArray textArray = json["Texts"].toArray();
        for (auto jObjRef : textArray) {
            auto* pText = new QGraphicsTextItem();
            read(pText, jObjRef.toObject());
            addItem(pText);
        }
    }
    if (json.contains("Rects") && json["Rects"].isArray()) {
        QJsonArray rectArray = json["Rects"].toArray();
        for (auto jObjRef : rectArray) {
            auto* pRect = new QGraphicsRectItem();
            read(pRect, jObjRef.toObject());
            addItem(pRect);
        }
    }
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
    jObject["fontName"] = textItem->font().family();
    jObject["fontSize"] = textItem->font().pointSize();
    jObject["positionX"] = textItem->pos().x();
    jObject["positionY"] = textItem->pos().y();
    jObject["scale"] = textItem->scale();
}

void MyScene::read(QGraphicsRectItem* rectItem, const QJsonObject& jObject)
{
    if (jObject.contains("positionX") && jObject["positionX"].isDouble() &&
        jObject.contains("positionY") && jObject["positionY"].isDouble()) {
        rectItem->setPos(jObject["positionX"].toDouble(), jObject["positionY"].toDouble());
    }
    if (jObject.contains("width") && jObject["width"].isDouble() &&
        jObject.contains("height") && jObject["height"].isDouble()) {
        rectItem->setRect(0, 0, jObject["width"].toDouble(), jObject["height"].toDouble());
    }
    if (jObject.contains("penColor") && jObject["penColor"].isDouble() &&
        jObject.contains("penWidth") && jObject["penWidth"].isDouble()) {
        QPen pen;
        
        int argb = jObject["penColor"].toInt();
        pen.setColor(QColor(QRgb((QRgb)argb)));
        pen.setWidth(jObject["penWidth"].toInt());
        rectItem->setPen(pen);
    }
    if (jObject.contains("brushColor") && jObject["brushColor"].isDouble()) {

        int argb = jObject["brushColor"].toInt();
        QColor color(qRed(argb), qGreen(argb), qBlue(argb), qAlpha(argb));
        QBrush brush(color);
        rectItem->setBrush(brush);
    }
    if (jObject.contains("scale") && jObject["scale"].isDouble()) {
        rectItem->setScale(jObject["scale"].toDouble());
    }
    rectItem->setFlag(QGraphicsItem::ItemIsMovable);
    rectItem->setFlag(QGraphicsItem::ItemIsSelectable);
}

void MyScene::read(QGraphicsTextItem* textItem, const QJsonObject& jObject)
{
    if (jObject.contains("text") && jObject["text"].isString()) {
        textItem->setPlainText(jObject["text"].toString());
    }
    if (jObject.contains("fontName") && jObject["fontName"].isString() &&
        jObject.contains("fontSize") && jObject["fontSize"].isDouble()) {
        textItem->setFont(QFont(jObject["fontName"].toString(), jObject["fontSize"].toInt()));
    }
    if (jObject.contains("positionX") && jObject["positionX"].isDouble() &&
        jObject.contains("positionY") && jObject["positionY"].isDouble()) {
        textItem->setPos(jObject["positionX"].toDouble(), jObject["positionY"].toDouble());
    }
    if (jObject.contains("scale") && jObject["scale"].isDouble()) {
        textItem->setScale(jObject["scale"].toDouble());
    }
    textItem->setFlag(QGraphicsItem::ItemIsMovable);
    textItem->setFlag(QGraphicsItem::ItemIsSelectable);
}
