#include "Commands.h"

#include <opencv2/opencv.hpp>
#include <qjsondocument.h>
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
    for (auto* myItem : myItems) {
        if (!myItem->scene())
            delete myItem;
    }
}

void AddCommand::undo()
{
    for (auto* myItem : myItems) {
        myGraphicsScene->removeItem(myItem);
    }
    myGraphicsScene->update();
}

void AddCommand::redo()
{
    for (auto* myItem : myItems) {
        myGraphicsScene->addItem(myItem);
        myItem->setPos(initialPosition);
    }
    myGraphicsScene->clearSelection();
    myGraphicsScene->update();
}

AddBoxCommand::AddBoxCommand(QGraphicsItem* box, QGraphicsScene* graphicsScene, QUndoCommand* parent) : AddCommand(graphicsScene, parent)
{
    auto* myItem = box;
    myItems.append(myItem);
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
    auto* myItem = qimage;
    myItems.append(myItem);
    setText(QObject::tr("Add %1")
        .arg(createCommandString(myItem, initialPosition)));
}

AddPasteCommand::AddPasteCommand(QString clipboardContent, MyScene* graphicsScene, QUndoCommand* parent) : AddCommand(graphicsScene, parent), clipJson(clipboardContent)
{
    myItems = graphicsScene->fromJson(QJsonDocument::fromJson(static_cast<const QByteArray> (clipboardContent.toStdString().c_str())).object());
    setText("Paste clipboard");
}

ModifyBrightnessCommand::ModifyBrightnessCommand(MyPicture* qPicture, QGraphicsScene* graphicsScene, QUndoCommand* parent) : QUndoCommand(parent), myPicture(qPicture), myGraphicsScene(graphicsScene)
{
    using VoidAction = std::function<void()>;

    cv::TrackbarCallback callbackForTrackBar = [](int pos, void* userdata)
    {
        (*(VoidAction*)userdata)();
    };

    std::string windowNameGamma = "Helligkeit";
    // Gamma brightness:
    int gammaI = myPicture->getGamma() * 100.0;
    double gamma = gammaI / 100.0;
    cv::Mat lookUpTable(1, 256, CV_8U);
    uchar* p = lookUpTable.ptr();
    for (int i = 0; i < 256; ++i)
        p[i] = cv::saturate_cast<uchar>(pow(i / 255.0, gamma) * 255.0);

    cv::Mat picture = cv::imread(myPicture->getCurrPath());
    cv::Mat imageDestination;
    cv::LUT(picture, lookUpTable, imageDestination);
    VoidAction doGammaLUT = [&]() {
        gamma = gammaI / 100.0;
        for (int i = 0; i < 256; ++i)
            p[i] = cv::saturate_cast<uchar>(pow(i / 255.0, gamma) * 255.0);
        cv::LUT(picture, lookUpTable, imageDestination);
        imshow(windowNameGamma, imageDestination);
    };

    cv::namedWindow(windowNameGamma, cv::WINDOW_NORMAL);
    cv::createTrackbar("Gamma Helligkeit", windowNameGamma, &gammaI, 200, callbackForTrackBar, (void*)&doGammaLUT);
    oldGamma = myPicture->getGamma();
    auto wait_time = 1000;
    cv::imshow(windowNameGamma, imageDestination);
    myPicture->setGamma(1.0);
    while (cv::getWindowProperty(windowNameGamma, cv::WND_PROP_VISIBLE) >= 1) {
        auto keyCode = cv::waitKey(wait_time);
        if (keyCode == 27) { // Wait for ESC key stroke
            cv::destroyAllWindows();
            break;
        }
    }
    myPicture->setGamma(gamma);
    setText("Helligkeit anpassen");
    cv::destroyAllWindows();
}

void ModifyBrightnessCommand::undo()
{
    cv::Mat picture = cv::imread(myPicture->getCurrPath());
    double gamma = oldGamma;
    picture = OpenCVWrapper::GammaBrightness(picture, gamma);
    QImage qim = OpenCVWrapper::Mat2QImage(picture);
    myPicture->setPixmap(QPixmap::fromImage(qim));
    myGraphicsScene->update();
}

void ModifyBrightnessCommand::redo()
{
    cv::Mat picture = cv::imread(myPicture->getCurrPath());
    double gamma = myPicture->getGamma();
    picture = OpenCVWrapper::GammaBrightness(picture, gamma);
    QImage qim = OpenCVWrapper::Mat2QImage(picture);
    myPicture->setPixmap(QPixmap::fromImage(qim));
    myGraphicsScene->update();
}
