#pragma once

#include <QUndoCommand>
#include <qgraphicsitem.h>

class MoveCommand : public QUndoCommand
{
public:
    enum { Id = 1234 };

    MoveCommand(QGraphicsItem* diagramItem, const QPointF& oldPos,
        QUndoCommand* parent = nullptr);

    void undo() override;
    void redo() override;
    bool mergeWith(const QUndoCommand* command) override;
    int id() const override { return Id; }

private:
    QGraphicsItem* myItem;
    QPointF myOldPos;
    QPointF newPos;
};

class DeleteCommand : public QUndoCommand
{
public:
    explicit DeleteCommand(QGraphicsScene* graphicsScene, QUndoCommand* parent = nullptr);

    void undo() override;
    void redo() override;

private:
    QGraphicsItem* myItem;
    QGraphicsScene* myGraphicsScene;
};