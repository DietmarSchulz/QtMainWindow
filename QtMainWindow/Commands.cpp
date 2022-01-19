#include "Commands.h"

#include <qgraphicsscene.h>
#include <opencv2/opencv.hpp>
#include "OpenCVWrapper.h"
#include "MyPicture.h"

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

AddCommand::AddCommand(QGraphicsScene* scene, QUndoCommand* parent)
    : QUndoCommand(parent), myGraphicsScene(scene)
{
}

AddCommand::~AddCommand()
{
    if (!myItem->scene())
        delete myItem;
}

void AddCommand::undo()
{
    myGraphicsScene->removeItem(myItem);
    myGraphicsScene->update();
}

void AddCommand::redo()
{
    myGraphicsScene->addItem(myItem);
    myItem->setPos(initialPosition);
    myGraphicsScene->clearSelection();
    myGraphicsScene->update();
}

AddBoxCommand::AddBoxCommand(QGraphicsItem* box, QGraphicsScene* graphicsScene, QUndoCommand* parent) : AddCommand(graphicsScene, parent)
{
    myItem = box;
    setText(QObject::tr("Add %1")
        .arg(createCommandString(myItem, initialPosition)));
    initialPosition = QPointF((itemCount * 15) % int(graphicsScene->width()),
        (itemCount * 15) % int(graphicsScene->height()));
    ++itemCount;
    myGraphicsScene->update();
}

AddPictureCommand::AddPictureCommand(std::string path, QGraphicsScene* graphicsScene, QUndoCommand* parent) : AddCommand(graphicsScene, parent)
{
    cv::Mat picture = cv::imread(path);
    QImage qim = OpenCVWrapper::Mat2QImage(picture);
    auto* qimage = new MyPicture(path, QPixmap::fromImage(qim));
    qimage->setFlag(QGraphicsItem::ItemIsMovable);
    qimage->setFlag(QGraphicsItem::ItemIsSelectable);
    qimage->setScale(0.25);
    myItem = qimage;
    setText(QObject::tr("Add %1")
        .arg(createCommandString(myItem, initialPosition)));
}

AddPasteCommand::AddPasteCommand(QString clipboardContent, QGraphicsScene* graphicsScene, QUndoCommand* parent) : AddCommand(graphicsScene, parent), clipJson(clipboardContent)
{
}

AddPasteCommand::~AddPasteCommand()
{
}

void AddPasteCommand::undo()
{
}

void AddPasteCommand::redo()
{
}
