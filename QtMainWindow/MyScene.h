#pragma once

#include <QObject>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <qundostack.h>
#include <QRubberBand>
#include "MyPicture.h"

class MyScene : public QGraphicsScene
{
    Q_OBJECT

public:
    MyScene(QUndoStack& undoSt, QObject* parent = nullptr);
    void load(QString& loadPath);
    void New();
    bool save();
    bool save(QString& savePath);

    void read(const QJsonObject& json);
    void write(QJsonObject& json) const;
    bool isModified();
    void SetModified(bool val);
    void SetView(QGraphicsView* view) { pview = view; }
    static QList<QGraphicsItem*> fromJson(const QJsonObject& json);
    static QJsonObject toJson(const QList<QGraphicsItem*>& selectedItems);
signals:
    void itemMoved(QList<QGraphicsItem*>& movedItem, std::vector<QPointF>& movedFromPosition);
    void message(QString);                                  // Text message signal
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;

private:
    QGraphicsView* pview;
    QGraphicsItem* movingItem = nullptr;
    QPointF oldPos;
    std::vector<QPointF> oldPositions;
    QString filePath;
    bool modified;
    QUndoStack& undoStack;
    std::unique_ptr<QRubberBand> rubberBand;
    QPointF origin;
    QPoint upperLeft{ 10, 67 };

    static void write(const QGraphicsRectItem* rectItem, QJsonObject& jObject);
    static void write(const QGraphicsTextItem* textItem, QJsonObject& jObject);
    static void read(QGraphicsRectItem* rectItem, const QJsonObject& jObject);
    static void read(QGraphicsTextItem* textItem, const QJsonObject& jObject);
};

