#pragma once

#include <QUndoCommand>
#include <qgraphicsitem.h>
#include "MyScene.h"

class MoveCommand : public QUndoCommand
{
public:
    enum { Id = 1234 };

    MoveCommand(QList<QGraphicsItem*>& items, std::vector<QPointF>& oldPositions,
        QUndoCommand* parent = nullptr);

    void undo() override;
    void redo() override;
    bool mergeWith(const QUndoCommand* command) override;
    int id() const override { return Id; }

private:
    QList<QGraphicsItem*> myItem;
    std::vector<QPointF> myOldPos;
    std::vector<QPointF> newPos;
};

class ScaleCommand : public QUndoCommand
{
public:
    enum { Id = 5678 };

    ScaleCommand(QGraphicsItem* item, double oldScale,
        QUndoCommand* parent = nullptr);

    void undo() override;
    void redo() override;
    bool mergeWith(const QUndoCommand* command) override;
    int id() const override { return Id; }

private:
    QGraphicsItem* myItem;
    double myOldScale;
    double newScale;
};

class RotateCommand : public QUndoCommand
{
public:
    enum { Id = 5678 };

    RotateCommand(QGraphicsItem* item, double oldRotation,
        QUndoCommand* parent = nullptr);

    void undo() override;
    void redo() override;
    bool mergeWith(const QUndoCommand* command) override;
    int id() const override { return Id; }

private:
    QGraphicsItem* myItem;
    double myOldRotation;
    double newRotation;
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
    enum { Id = 6891 };
    ModifyBrightnessCommand(MyPicture* qPicture, QGraphicsScene* graphicsScene, QUndoCommand* parent = nullptr);
    void undo() override;
    void redo() override;

protected:
    ModifyBrightnessCommand(QGraphicsScene* graphicsScene, MyPicture* qPicture, QUndoCommand* parent = nullptr) : QUndoCommand(parent), myPicture(qPicture), myGraphicsScene(graphicsScene) {};
    QGraphicsScene* myGraphicsScene;
    MyPicture* myPicture;
    double oldGamma;
    double newGamma;
};

class SetBrightnessCommand : public ModifyBrightnessCommand
{
public:
    enum { Id = 7891 };
    SetBrightnessCommand(double gamma, MyPicture* qPicture, QGraphicsScene* graphicsScene, QUndoCommand* parent = nullptr);
    bool mergeWith(const QUndoCommand* command) override;
    int id() const override { return Id; }
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

class ModifyRGBScaleCommand : public QUndoCommand
{
public:
    ModifyRGBScaleCommand(MyPicture* qPicture, QGraphicsScene* graphicsScene, QUndoCommand* parent = nullptr);
    void undo() override;
    void redo() override;

private:
    QGraphicsScene* myGraphicsScene;
    MyPicture* myPicture;
    double oldGammaRed;
    double oldGammaGreen;
    double oldGammaBlue;
    double newGammaRed;
    double newGammaGreen;
    double newGammaBlue;
};

class AddPicturesCommand : public QUndoCommand
{
public:
    AddPicturesCommand(MyPicture* qPicture, MyPicture* secondPicture, QGraphicsScene* graphicsScene, QUndoCommand* parent = nullptr);
    void undo() override;
    void redo() override;

private:
    QGraphicsScene* myGraphicsScene;
    MyPicture* myPicture;
    double oldAlphaAdd;
    double newAlphaAdd;
    std::string oldSecondPath;
    std::string newSecondPath;
};
