#include "Commands.h"

#include <opencv2/opencv.hpp>
#include <qjsondocument.h>
#include <qtextdocument.h>
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
    setText(QObject::tr("L�sche %1")
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
    newGamma = gamma;
    setText("Helligkeit anpassen");
    cv::destroyAllWindows();
}

void ModifyBrightnessCommand::undo()
{
    cv::Mat picture = cv::imread(myPicture->getCurrPath());
    double gamma = oldGamma;
    myPicture->setGamma(gamma);
    picture = OpenCVWrapper::GammaBrightness(picture, gamma);
    QImage qim = OpenCVWrapper::Mat2QImage(picture);
    myPicture->setPixmap(QPixmap::fromImage(qim));
    myGraphicsScene->update();
}

void ModifyBrightnessCommand::redo()
{
    cv::Mat picture = cv::imread(myPicture->getCurrPath());
    double gamma = newGamma;
    myPicture->setGamma(gamma);
    picture = OpenCVWrapper::GammaBrightness(picture, gamma);
    QImage qim = OpenCVWrapper::Mat2QImage(picture);
    myPicture->setPixmap(QPixmap::fromImage(qim));
    myGraphicsScene->update();
}

ChangeTextCommand::ChangeTextCommand(QGraphicsTextItem* newItem, QUndoCommand* parent) : firstTime(true), myItem(newItem)
{
    setText(reinterpret_cast<const char*>(u8"Text�nderung"));
}

void ChangeTextCommand::undo()
{
    auto* doc = myItem->document();
    doc->undo();
}

void ChangeTextCommand::redo()
{
    if (firstTime) {
        firstTime = false;
        return;
    }
    auto* doc = myItem->document();
    doc->redo();
}

ModifyRGBScaleCommand::ModifyRGBScaleCommand(MyPicture* qPicture, QGraphicsScene* graphicsScene, QUndoCommand* parent) : QUndoCommand(parent), myPicture(qPicture), myGraphicsScene(graphicsScene)
{
    std::string redWindowName{ "Rotanteil" };
    cv::namedWindow(redWindowName, cv::WINDOW_NORMAL);
    cv::moveWindow(redWindowName, 400, 0);
    std::string greenWindowName{ "Gruenanteil" };
    cv::namedWindow(greenWindowName, cv::WINDOW_NORMAL);
    cv::moveWindow(greenWindowName, 800, 0);
    std::string blueWindowName{ "Blauanteil" };
    cv::namedWindow(blueWindowName, cv::WINDOW_NORMAL);
    cv::moveWindow(blueWindowName, 1200, 0);

    cv::Mat orgImg = cv::imread(myPicture->getCurrPath());
    std::vector<cv::Mat> bgr;
    split(orgImg, bgr);

    cv::Mat blueDummy;
    cv::Mat greenDummy;
    cv::Mat redDummy;
    cv::Mat z = cv::Mat::zeros(bgr[0].size(), bgr[0].type());
    std::vector<cv::Mat> dummy{ z, z, z };

    using VoidAction = std::function<void()>;

    // Gamma brightnesses for rgb:
    oldGammaRed = myPicture->getGammaRed();
    newGammaRed = oldGammaRed;
    int redGammaI = 100 * oldGammaRed;
    cv::Mat redlookUpTable(1, 256, CV_8U);
    uchar* redp = redlookUpTable.ptr();
    oldGammaGreen = myPicture->getGammaGreen();
    int greenGammaI = 100 * oldGammaGreen;
    cv::Mat greenlookUpTable(1, 256, CV_8U);
    uchar* greenp = greenlookUpTable.ptr();
    oldGammaBlue = myPicture->getGammaBlue();
    int blueGammaI = 100 * oldGammaBlue;
    cv::Mat bluelookUpTable(1, 256, CV_8U);
    uchar* bluep = bluelookUpTable.ptr();

    std::string resultWindowName{ "Mischergebnis" };
    cv::namedWindow(resultWindowName, cv::WINDOW_NORMAL);
    cv::moveWindow(resultWindowName, 400, 400);
    VoidAction doGammaLUT = [&]() {
        newGammaBlue = blueGammaI / 100.0;
        for (int i = 0; i < 256; ++i)
            bluep[i] = cv::saturate_cast<uchar>(pow(i / 255.0, newGammaBlue) * 255.0);
        LUT(bgr[0], bluelookUpTable, bgr[0]);
        dummy[2] = z;
        dummy[0] = bgr[0];
        merge(dummy, blueDummy);
        imshow(blueWindowName, blueDummy);

        newGammaGreen = greenGammaI / 100.0;
        for (int i = 0; i < 256; ++i)
            greenp[i] = cv::saturate_cast<uchar>(pow(i / 255.0, newGammaGreen) * 255.0);
        LUT(bgr[1], greenlookUpTable, bgr[1]);
        dummy[0] = z;
        dummy[1] = bgr[1];
        merge(dummy, greenDummy);
        imshow(greenWindowName, greenDummy);

        newGammaRed = redGammaI / 100.0;
        for (int i = 0; i < 256; ++i)
            redp[i] = cv::saturate_cast<uchar>(pow(i / 255.0, newGammaRed) * 255.0);
        LUT(bgr[2], redlookUpTable, bgr[2]);
        dummy[1] = z;
        dummy[2] = bgr[2];
        merge(dummy, redDummy);
        imshow(redWindowName, redDummy);

        cv::Mat result;
        merge(bgr, result);
        imshow(resultWindowName, result);

        // For next round
        split(orgImg, bgr);
    };

    cv::TrackbarCallback callbackForTrackBars = [](int pos, void* userdata)
    {
        (*(VoidAction*)userdata)();
    };

    auto intVal{ 0 };
    callbackForTrackBars(intVal, (void*)&doGammaLUT);

    cv::createTrackbar("Gamma rot", redWindowName, &redGammaI, 200, callbackForTrackBars, (void*)&doGammaLUT);
    cv::createTrackbar("Gamma gruen", greenWindowName, &greenGammaI, 200, callbackForTrackBars, (void*)&doGammaLUT);
    cv::createTrackbar("Gamma blau", blueWindowName, &blueGammaI, 200, callbackForTrackBars, (void*)&doGammaLUT);

    auto wait_time = 1000;
    while (cv::getWindowProperty(resultWindowName, cv::WND_PROP_VISIBLE) >= 1) {
        auto keyCode = cv::waitKey(wait_time);
        if (keyCode == 27) { // Wait for ESC key stroke
            cv::destroyAllWindows();
            break;
        }
    }

    cv::destroyAllWindows(); //destroy all open windows
}

void ModifyRGBScaleCommand::undo()
{
    cv::Mat picture = cv::imread(myPicture->getCurrPath());
    myPicture->setGammaRed(oldGammaRed);
    myPicture->setGammaGreen(oldGammaGreen);
    myPicture->setGammaBlue(oldGammaBlue);
    picture = OpenCVWrapper::ScaleRGB(picture, oldGammaRed, oldGammaGreen, oldGammaBlue);
    QImage qim = OpenCVWrapper::Mat2QImage(picture);
    myPicture->setPixmap(QPixmap::fromImage(qim));
    myGraphicsScene->update();
}

void ModifyRGBScaleCommand::redo()
{
    cv::Mat picture = cv::imread(myPicture->getCurrPath());
    myPicture->setGammaRed(newGammaRed);
    myPicture->setGammaGreen(newGammaGreen);
    myPicture->setGammaBlue(newGammaBlue);
    picture = OpenCVWrapper::ScaleRGB(picture, newGammaRed, newGammaGreen, newGammaBlue);
    QImage qim = OpenCVWrapper::Mat2QImage(picture);
    myPicture->setPixmap(QPixmap::fromImage(qim));
    myGraphicsScene->update();
}
