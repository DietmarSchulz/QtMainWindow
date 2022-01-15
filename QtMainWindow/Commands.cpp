#include "Commands.h"

#include <qgraphicsscene.h>

QString createCommandString(QGraphicsItem* item, const QPointF& pos)
{
    return QObject::tr("%1 at (%2, %3)")
        .arg(item->type() == QGraphicsPixmapItem::Type ? "Picture" : "Rect/Text")
        .arg(pos.x()).arg(pos.y());
}

//! [0]
MoveCommand::MoveCommand(QGraphicsItem* QGraphicsItem, const QPointF& oldPos,
    QUndoCommand* parent)
    : QUndoCommand(parent), myItem(QGraphicsItem)
    , myOldPos(oldPos), newPos(QGraphicsItem->pos())
{
}
//! [0]

//! [1]
bool MoveCommand::mergeWith(const QUndoCommand* command)
{
    const MoveCommand* moveCommand = static_cast<const MoveCommand*>(command);
    QGraphicsItem* item = moveCommand->myItem;

    if (myItem != item)
        return false;

    newPos = item->pos();
    setText(QObject::tr("Move %1")
        .arg(createCommandString(myItem, newPos)));

    return true;
}
//! [1]

//! [2]
void MoveCommand::undo()
{
    myItem->setPos(myOldPos);
    myItem->scene()->update();
    setText(QObject::tr("Move %1")
        .arg(createCommandString(myItem, newPos)));
}
//! [2]

//! [3]
void MoveCommand::redo()
{
    myItem->setPos(newPos);
    setText(QObject::tr("Move %1")
        .arg(createCommandString(myItem, newPos)));
}
//! [3]
