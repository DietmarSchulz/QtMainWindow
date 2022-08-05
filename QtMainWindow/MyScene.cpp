#include "MyScene.h"
#include <QGraphicsSceneMouseEvent>
#include <QJsonArray>
#include <QtCore/qfile.h>
#include <qjsondocument.h>

MyScene::MyScene(QUndoStack& undoSt, QObject* parent)
    : QGraphicsScene(parent), modified(false), undoStack(undoSt)
{
    setSceneRect(-800, -800, 2200, 3000);
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
    modified = false;
}

void MyScene::New()
{
    clear();
    filePath.clear();
    modified = false;
}

bool MyScene::save()
{
    if (filePath.isEmpty()) {
        return false;
    }
    modified = false;
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
    modified = false;
    return true;
}

void MyScene::read(const QJsonObject& json)
{
    auto currItems = fromJson(json);
    for (auto* it : currItems) {
        addItem(it);
    }
}

void MyScene::write(QJsonObject& json) const
{
    json = toJson(items());
}

bool MyScene::isModified()
{
    return modified;
}

void MyScene::SetModified(bool val)
{
    modified = val;
}

QList<QGraphicsItem*> MyScene::fromJson(const QJsonObject& json)
{
    QList<QGraphicsItem*> selectedJson;
    if (json.contains("Pictures") && json["Pictures"].isArray()) {
        QJsonArray pictureArray = json["Pictures"].toArray();
        for (auto jObjRef : pictureArray) {
            auto* pPicture = new MyPicture();
            pPicture->read(jObjRef.toObject());
            selectedJson.append(pPicture);
        }
    }
    if (json.contains("Texts") && json["Texts"].isArray()) {
        QJsonArray textArray = json["Texts"].toArray();
        for (auto jObjRef : textArray) {
            auto* pText = new QGraphicsTextItem();
            read(pText, jObjRef.toObject());
            selectedJson.append(pText);
        }
    }
    if (json.contains("Rects") && json["Rects"].isArray()) {
        QJsonArray rectArray = json["Rects"].toArray();
        for (auto jObjRef : rectArray) {
            auto* pRect = new QGraphicsRectItem();
            read(pRect, jObjRef.toObject());
            selectedJson.append(pRect);
        }
    }    return selectedJson;
}

void MyScene::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    QPointF mousePos(event->buttonDownScenePos(Qt::LeftButton).x(),
        event->buttonDownScenePos(Qt::LeftButton).y());
    const QList<QGraphicsItem*> itemList = items(mousePos);
    movingItem = itemList.isEmpty() ? nullptr : itemList.first();

    if (movingItem != nullptr && event->button() == Qt::LeftButton) {
        oldPos = movingItem->pos();
        auto sel = selectedItems();

        oldPositions.clear();
        if (!sel.isEmpty() && sel.contains(movingItem)) {
            for (auto& s : sel) {
                oldPositions.push_back(s->pos());
            }
        }
        else {
            oldPositions.push_back(oldPos);
        }
        modified = true;
    }
    else if (event->button() == Qt::LeftButton) {
        QWidget* parentWidget = static_cast<QWidget*>(parent());
        origin = event->scenePos();
        auto start = pview->mapFromScene(origin.toPoint()) + QPoint(10, 67);
        if (!rubberBand)
            rubberBand = std::make_unique<QRubberBand>(QRubberBand::Rectangle, parentWidget);
        rubberBand->setGeometry(QRect(start, QSize()));
        if (selectedItems().count() == 0) {
            rubberBand->show();
        }
    }

    QGraphicsScene::mousePressEvent(event);
}

void MyScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    QPointF mousePos(event->buttonDownScenePos(Qt::LeftButton).x(),
        event->buttonDownScenePos(Qt::LeftButton).y());
    if (movingItem != nullptr && event->button() == Qt::LeftButton) {
        if (oldPos != movingItem->pos()) {
            auto sel = selectedItems();
            emit itemMoved(sel,
                oldPositions);
            clearSelection();
        }
        movingItem = nullptr;
        emit message("Items moved");
    }
    else {
        if (rubberBand) {
            if (rubberBand->isVisible()) {
                auto rect = QRect (pview->mapToScene(rubberBand->rect().topLeft() - QPoint(10, 67)).toPoint(), pview->mapToScene(rubberBand->rect().bottomRight() - QPoint(10, 67)).toPoint());
                clearSelection();
                for (auto& item : items()) {
                    if (rect.contains(item->scenePos().toPoint())) {
                        item->setSelected(true);
                    }
                }
            }
            rubberBand->hide();
        }

    }
    QGraphicsScene::mouseReleaseEvent(event);
}

void MyScene::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    if (rubberBand) {
        auto start = pview->mapFromScene(origin.toPoint()) + QPoint(10, 67);
        auto end = pview->mapFromScene(event->scenePos().toPoint()) + QPoint(10, 67);
        rubberBand->setGeometry(QRect(start,
            end).normalized());
    }
    QGraphicsScene::mouseMoveEvent(event);
}

void MyScene::write(const QGraphicsRectItem* rectItem, QJsonObject& jObject)
{
    jObject["positionX"] = rectItem->pos().x();
    jObject["positionY"] = rectItem->pos().y();
    jObject["width"] = rectItem->rect().width();
    jObject["height"] = rectItem->rect().height();
    jObject["penColor"] = (int) rectItem->pen().color().rgba();
    jObject["penWidth"] = rectItem->pen().width();
    jObject["brushColor"] = (int) rectItem->brush().color().rgba();
    jObject["scale"] = rectItem->scale();
    if (rectItem->zValue() != 0.0) {
        jObject["zvalue"] = rectItem->zValue();
    }
    jObject["rotation"] = rectItem->rotation();
}

void MyScene::write(const QGraphicsTextItem* textItem, QJsonObject& jObject)
{
    jObject["text"] = textItem->toPlainText();
    jObject["fontName"] = textItem->font().family();
    jObject["fontSize"] = textItem->font().pointSize();
    jObject["positionX"] = textItem->pos().x();
    jObject["positionY"] = textItem->pos().y();
    jObject["scale"] = textItem->scale();
    if (textItem->zValue() != 0.0) {
        jObject["zvalue"] = textItem->zValue();
    }
    jObject["rotation"] = textItem->rotation();
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
        
        int argb = jObject["penColor"].toInt();
        QColor color(qRed(argb), qGreen(argb), qBlue(argb), qAlpha(argb));
        QPen pen(color);
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
    if (jObject.contains("zvalue") && jObject["zvalue"].isDouble()) {
        rectItem->setZValue(jObject["zvalue"].toDouble());
    }
    if (jObject.contains("rotation") && jObject["rotation"].isDouble()) {
        rectItem->setRotation(jObject["rotation"].toDouble());
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
    if (jObject.contains("zvalue") && jObject["zvalue"].isDouble()) {
        textItem->setZValue(jObject["zvalue"].toDouble());
    }
    if (jObject.contains("rotation") && jObject["rotation"].isDouble()) {
        textItem->setRotation(jObject["rotation"].toDouble());
    }
    textItem->setFlag(QGraphicsItem::ItemIsMovable);
    textItem->setFlag(QGraphicsItem::ItemIsSelectable);
    textItem->setFlag(QGraphicsItem::ItemIsPanel);
    textItem->setTextInteractionFlags(Qt::TextInteractionFlag::TextEditorInteraction);
}

QJsonObject MyScene::toJson(const QList<QGraphicsItem*>& selectedItems)
{
    QJsonObject jsonSelection;
    QJsonArray jPictures;
    QJsonArray jTexts;
    QJsonArray jRects;
    for (const auto* it : selectedItems) {
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
    jsonSelection["Pictures"] = jPictures;
    jsonSelection["Texts"] = jTexts;
    jsonSelection["Rects"] = jRects;
    return jsonSelection;
}
