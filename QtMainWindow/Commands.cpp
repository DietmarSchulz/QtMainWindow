#include "Commands.h"

#include <qgraphicsscene.h>

QString createCommandString(QGraphicsItem* item, const QPointF& pos)
{
    return QObject::tr("%1 at (%2, %3)")
        .arg(item->type() == QGraphicsPixmapItem::Type ? "Picture" : "Rect/Text")
        .arg(pos.x()).arg(pos.y());
}

MoveCommand::MoveCommand(QGraphicsItem* QGraphicsItem, const QPointF& oldPos,
    QUndoCommand* parent)
    : QUndoCommand(parent), myItem(QGraphicsItem)
    , myOldPos(oldPos), newPos(QGraphicsItem->pos())
{
}

bool MoveCommand::mergeWith(const QUndoCommand* command)
{
    const MoveCommand* moveCommand = static_cast<const MoveCommand*>(command);
    QGraphicsItem* item = moveCommand->myItem;

    if (myItem != item)
        return false;

    newPos = item->pos();
    setText(QObject::tr("Bewege %1")
        .arg(createCommandString(myItem, newPos)));

    return true;
}

void MoveCommand::undo()
{
    myItem->setPos(myOldPos);
    myItem->scene()->update();
    setText(QObject::tr("Bewege %1")
        .arg(createCommandString(myItem, newPos)));
}

void MoveCommand::redo()
{
    myItem->setPos(newPos);
    setText(QObject::tr("Move %1")
        .arg(createCommandString(myItem, newPos)));
}

DeleteCommand::DeleteCommand(QGraphicsScene* scene, QUndoCommand* parent)
    : QUndoCommand(parent), myGraphicsScene(scene)
{
    QList<QGraphicsItem*> list = myGraphicsScene->selectedItems();
    list.first()->setSelected(false);
    myItem = list.first();
    setText(QObject::tr("Lösche %1")
        .arg(createCommandString(myItem, myItem->pos())));
}

void DeleteCommand::undo()
{
    myGraphicsScene->addItem(myItem);
    myGraphicsScene->update();
}

void DeleteCommand::redo()
{
    myGraphicsScene->removeItem(myItem);
}
