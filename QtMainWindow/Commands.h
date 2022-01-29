#pragma once

#include <QUndoCommand>
#include <qgraphicsitem.h>
#include "MyScene.h"

class MoveCommand : public QUndoCommand
{
public:
    MoveCommand(QGraphicsItem* diagramItem, const QPointF& oldPos,
        QUndoCommand* parent = nullptr);

    void undo() override;
    void redo() override;
    bool mergeWith(const QUndoCommand* command) override;

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

class AddCommand : public QUndoCommand
{
protected:
    AddCommand(QGraphicsScene* graphicsScene,
        QUndoCommand* parent = nullptr);
public:
    virtual ~AddCommand();

    void undo() override;
    void redo() override;

protected:
    inline static int itemCount{0};
    QList<QGraphicsItem*> myItems;
    QGraphicsScene* myGraphicsScene;
    QPointF initialPosition;
};

class AddBoxCommand : public AddCommand
{
public:
    AddBoxCommand(QGraphicsItem* box, QGraphicsScene* graphicsScene,
        QUndoCommand* parent = nullptr);
};

class AddPictureCommand : public AddCommand
{
public:
    AddPictureCommand(std::string path, QGraphicsScene* graphicsScene,
        QUndoCommand* parent = nullptr);
};

class AddPasteCommand : public AddCommand
{
    QString clipJson;
public:
    AddPasteCommand(QString clipboardContent, MyScene* graphicsScene,
        QUndoCommand* parent = nullptr);
};

class ModifyBrightnessCommand : public QUndoCommand
{
public:
    ModifyBrightnessCommand(MyPicture* qPicture, QGraphicsScene* graphicsScene, QUndoCommand* parent = nullptr);
    void undo() override;
    void redo() override;

private:
    QGraphicsScene* myGraphicsScene;
    MyPicture* myPicture;
    double oldGamma;
};

class ChangeTextCommand : public QUndoCommand
{
public:
    ChangeTextCommand(QGraphicsTextItem* newItem,
        QUndoCommand* parent = nullptr);

    void undo() override;
    void redo() override;

private:
    QGraphicsTextItem* myItem;
    bool firstTime;
};
