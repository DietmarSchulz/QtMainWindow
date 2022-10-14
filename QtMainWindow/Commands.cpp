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

MoveCommand::MoveCommand(QList<QGraphicsItem*>& items, std::vector<QPointF>& oldPositions,
    QUndoCommand* parent)
    : QUndoCommand(parent), myItem(items), myOldPos(oldPositions)
{
    for (auto gr : items) {
        newPos.push_back(gr->pos());
    }
}

bool MoveCommand::mergeWith(const QUndoCommand* command)
{
    const MoveCommand* moveCommand = static_cast<const MoveCommand*>(command);
    auto& items = moveCommand->myItem;

    if (myItem != items)
        return false;

    for (auto i = 0;  auto s : items) {
        newPos[i] = s->pos();
        i++;
    }
    setText(QObject::tr("Bewege %1")
        .arg(createCommandString(myItem[0], newPos[0])));

    return true;
}

void MoveCommand::undo()
{
    for (auto i = 0; auto* it : myItem) {
        it->setPos(myOldPos[i]);
        it->scene()->update();
        i++;
    }
    setText(QObject::tr("Bewege %1")
        .arg(createCommandString(myItem[0], newPos[0])));
}

void MoveCommand::redo()
{
    for (auto i = 0; auto * it : myItem) {
        it->setPos(newPos[i]);
        it->scene()->update();
        i++;
    }
    setText(QObject::tr("Bewege %1")
        .arg(createCommandString(myItem[0], newPos[0])));
}

ScaleCommand::ScaleCommand(QGraphicsItem* item, double oldScale, QUndoCommand* parent)
    : QUndoCommand(parent), myItem(item), myOldScale(oldScale)
{
    newScale = myItem->scale();
}

void ScaleCommand::undo()
{
    myItem->setScale(myOldScale);
    myItem->scene()->update();
    setText(QObject::tr("Skaliere %1 (%2)").arg(myItem->type()).arg(myOldScale));
}

void ScaleCommand::redo()
{
    myItem->setScale(newScale);
    myItem->scene()->update();
    setText(QObject::tr("Skaliere %1 (%2)").arg(myItem->type()).arg(newScale));
}

bool ScaleCommand::mergeWith(const QUndoCommand* command)
{
    const ScaleCommand* scaleCommand = static_cast<const ScaleCommand*>(command);
    auto& item = scaleCommand->myItem;

    if (myItem != item)
        return false;

    newScale = item->scale();
    setText(QObject::tr("Skaliere %1 (%2)").arg(myItem->type()).arg(newScale));
    return true;
}

ZvalueCommand::ZvalueCommand(QGraphicsItem* item, double newZvalue, QUndoCommand* parent) : QUndoCommand(parent), myItem(item), newZvalue(newZvalue)
{
    myOldZvalue = item->zValue();
    setText(QObject::tr("Setze Z-Value %1 (%2)").arg(myItem->type()).arg(newZvalue));
}

void ZvalueCommand::undo()
{
    myItem->setZValue(myOldZvalue);
}

void ZvalueCommand::redo()
{
    myItem->setZValue(newZvalue);
}

bool ZvalueCommand::mergeWith(const QUndoCommand* command)
{
    const ZvalueCommand* zvalueCommand = static_cast<const ZvalueCommand*>(command);
    auto& item = zvalueCommand->myItem;

    if (myItem != item)
        return false;

    newZvalue = item->zValue();
    setText(QObject::tr("Setze Z-Value %1 (%2)").arg(myItem->type()).arg(newZvalue));
    return true;
}

RotateCommand::RotateCommand(QGraphicsItem* item, double oldRotation, QUndoCommand* parent)
    : QUndoCommand(parent), myItem(item), myOldRotation(oldRotation)
{
    newRotation = item->rotation();
}

void RotateCommand::undo()
{
    myItem->setRotation(myOldRotation);
    myItem->scene()->update();
    setText(QObject::tr("Rotiere %1 (%2)").arg(myItem->type()).arg(myOldRotation));
}

void RotateCommand::redo()
{
    myItem->setRotation(newRotation);
    myItem->scene()->update();
    setText(QObject::tr("Rotiere %1 (%2)").arg(myItem->type()).arg(newRotation));
}

bool RotateCommand::mergeWith(const QUndoCommand* command)
{
    const RotateCommand* rotationCommand = static_cast<const RotateCommand*>(command);
    auto& item = rotationCommand->myItem;

    if (myItem != item)
        return false;

    newRotation = item->rotation();
    setText(QObject::tr("Rotiere %1 (%2)").arg(myItem->type()).arg(newRotation));
    return true;
}

DeleteCommand::DeleteCommand(QGraphicsScene* scene, QUndoCommand* parent)
    : QUndoCommand(parent), myGraphicsScene(scene)
{
    QList<QGraphicsItem*> list = myGraphicsScene->selectedItems();
    list.first()->setSelected(false);
    myItem = list.first();
    setText(QObject::tr("L\303\266sche %1")
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

AddBoxCommand::AddBoxCommand(QGraphicsItem* box, QGraphicsScene* graphicsScene, QPoint& startPos, QUndoCommand* parent) : AddCommand(graphicsScene, parent)
{
    auto* myItem = box;
    myItems.append(myItem);
    initialPosition = startPos;
    setText(QObject::tr("Add %1")
        .arg(createCommandString(myItem, initialPosition)));
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
    auto oldGammaRed = myPicture->getGammaRed();
    auto oldGammaGreen = myPicture->getGammaGreen();
    auto oldGammaBlue = myPicture->getGammaBlue();
    if (oldGammaRed != 1.0 || oldGammaGreen != 1.0 || oldGammaBlue != 1.0) {
        picture = OpenCVWrapper::ScaleRGB(picture, oldGammaRed, oldGammaGreen, oldGammaBlue);
    }
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
    auto oldGammaRed = myPicture->getGammaRed();
    auto oldGammaGreen = myPicture->getGammaGreen();
    auto oldGammaBlue = myPicture->getGammaBlue();
    if (oldGammaRed != 1.0 || oldGammaGreen != 1.0 || oldGammaBlue != 1.0) {
        picture = OpenCVWrapper::ScaleRGB(picture, oldGammaRed, oldGammaGreen, oldGammaBlue);
    }
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
    auto oldGammaRed = myPicture->getGammaRed();
    auto oldGammaGreen = myPicture->getGammaGreen();
    auto oldGammaBlue = myPicture->getGammaBlue();
    if (oldGammaRed != 1.0 || oldGammaGreen != 1.0 || oldGammaBlue != 1.0) {
        picture = OpenCVWrapper::ScaleRGB(picture, oldGammaRed, oldGammaGreen, oldGammaBlue);
    }
    QImage qim = OpenCVWrapper::Mat2QImage(picture);
    myPicture->setPixmap(QPixmap::fromImage(qim));
    myGraphicsScene->update();
}

SetBrightnessCommand::SetBrightnessCommand(double gamma, MyPicture* qPicture, QGraphicsScene* graphicsScene, QUndoCommand* parent) : ModifyBrightnessCommand(graphicsScene, qPicture, parent)
{
    newGamma = gamma;
    oldGamma = myPicture->getGamma();
    setText("Helligkeit setzen: " + QString::number(newGamma));
}

bool SetBrightnessCommand::mergeWith(const QUndoCommand* command)
{
    const SetBrightnessCommand* brightnessCommand = static_cast<const SetBrightnessCommand*>(command);
    auto& picToMerge = brightnessCommand->myPicture;
    if (myPicture != picToMerge)
        return false;
    newGamma = brightnessCommand->newGamma;
    setText("Helligkeit setzen: " + QString::number(newGamma));
    return true;
}

ChangeTextCommand::ChangeTextCommand(QGraphicsTextItem* newItem, QUndoCommand* parent) : firstTime(true), myItem(newItem)
{
    setText(reinterpret_cast<const char*>(u8"Textänderung"));
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

    // using brightness gamma as predecessor
    auto brightnessGamma = myPicture->getGamma();
    orgImg = OpenCVWrapper::GammaBrightness(orgImg, brightnessGamma);
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

    setText("RGB anpassen");
    cv::destroyAllWindows(); //destroy all open windows
}

void ModifyRGBScaleCommand::undo()
{
    cv::Mat picture = cv::imread(myPicture->getCurrPath());

    // using brightness gamma as predecessor
    auto brightnessGamma = myPicture->getGamma();
    picture = OpenCVWrapper::GammaBrightness(picture, brightnessGamma);
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

    // using brightness gamma as predecessor
    auto brightnessGamma = myPicture->getGamma();
    picture = OpenCVWrapper::GammaBrightness(picture, brightnessGamma);
    myPicture->setGammaRed(newGammaRed);
    myPicture->setGammaGreen(newGammaGreen);
    myPicture->setGammaBlue(newGammaBlue);
    picture = OpenCVWrapper::ScaleRGB(picture, newGammaRed, newGammaGreen, newGammaBlue);
    QImage qim = OpenCVWrapper::Mat2QImage(picture);
    myPicture->setPixmap(QPixmap::fromImage(qim));
    myGraphicsScene->update();
}

SetRGBScaleCommand::SetRGBScaleCommand(double gammaRed, double gammaGreen, double gammaBlue, MyPicture* qPicture, QGraphicsScene* graphicsScene, QUndoCommand* parent) : ModifyRGBScaleCommand(graphicsScene, qPicture, parent)
{
    newGammaRed = gammaRed;
    newGammaGreen = gammaGreen;
    newGammaBlue = gammaBlue;
    oldGammaRed = myPicture->getGammaRed();
    oldGammaGreen = myPicture->getGammaGreen();
    oldGammaBlue = myPicture->getGammaBlue();
    setText("RGB setzen: " + QString::number(newGammaRed + newGammaGreen + newGammaBlue));
}

bool SetRGBScaleCommand::mergeWith(const QUndoCommand* command)
{
    const SetRGBScaleCommand* setRGBCommand = static_cast<const SetRGBScaleCommand*>(command);
    auto& picToMerge = setRGBCommand->myPicture;
    if (myPicture != picToMerge)
        return false;
    newGammaRed = setRGBCommand->newGammaRed;
    newGammaGreen = setRGBCommand->newGammaGreen;
    newGammaBlue = setRGBCommand->newGammaBlue;
    return true;
}

AddPicturesCommand::AddPicturesCommand(MyPicture* qPicture, MyPicture* secondPicture, QGraphicsScene* graphicsScene, QUndoCommand* parent) : QUndoCommand(parent), myPicture(qPicture), myGraphicsScene(graphicsScene)
{
    oldAlphaAdd = myPicture->getAlphaAdd();
    newAlphaAdd = oldAlphaAdd;
    oldSecondPath = myPicture->getSecondPath();

    // store 2nd pics info:
    myPicture->setGammaSecond(secondPicture->getGamma());
    myPicture->setGammaRedSecond(secondPicture->getGammaRed());
    myPicture->setGammaGreenSecond(secondPicture->getGammaGreen());
    myPicture->setGammaBlueSecond(secondPicture->getGammaBlue());
    myPicture->setSecondPath(secondPicture->getCurrPath());

    // Some windows to control alphaAdd
    cv:: Mat firstImg = cv::imread(myPicture->getCurrPath());
    cv::Mat secondImg = cv::imread(secondPicture->getCurrPath());
    std::string firstOriginalWindowName{ "First original" };
    std::string secondOriginalWindowName{ "Second original" };
    cv::namedWindow(firstOriginalWindowName, cv::WINDOW_NORMAL);
    cv::moveWindow(firstOriginalWindowName, 0, 0);
    cv::namedWindow(secondOriginalWindowName, cv::WINDOW_NORMAL);
    cv::moveWindow(secondOriginalWindowName, 0, 320);
    imshow(firstOriginalWindowName, firstImg);
    imshow(secondOriginalWindowName, secondImg);
    if (firstImg.size() != secondImg.size()) {
        if (firstImg.size().width < secondImg.size().width && firstImg.size().height < secondImg.size().height) {
            resize(firstImg, firstImg, secondImg.size());
        }
        else if (firstImg.size().width > secondImg.size().width && firstImg.size().height > secondImg.size().height) {
            resize(secondImg, secondImg, firstImg.size());
        }
        else {
            resize(firstImg, firstImg, secondImg.size());
        }
    }

    double alpha = oldAlphaAdd; double beta;
    using VoidAction = std::function<void()>;
    std::string firstAndSecondAdded{ "First and second picture added" };

    //Create trackbar to change blend
    int iSliderValue1 = 0;
    cv::namedWindow(firstAndSecondAdded, cv::WINDOW_NORMAL);
    cv::moveWindow(firstAndSecondAdded, 400, 0);

    VoidAction doBlending = [&] {
        //Change the brightness and contrast of the image (For more infomation http://opencv-srf.blogspot.com/2013/07/change-contrast-of-image-or-video.html)
        cv::Mat dst;
        alpha = (double)iSliderValue1 / 100;

        beta = (1.0 - alpha);
        addWeighted(firstImg, alpha, secondImg, beta, 0.0, dst);

        //show the brightness and contrast adjusted image
        imshow(firstAndSecondAdded, dst);
    };

    cv::TrackbarCallback callbackForTrackBar = [](int pos, void* userdata)
    {
        (*(VoidAction*)userdata)();
    };
    cv::createTrackbar("Blend", firstAndSecondAdded, &iSliderValue1, 100, callbackForTrackBar, (void*)&doBlending);

    int iDummy{ 0 };
    callbackForTrackBar(iDummy, (void*)&doBlending);

    auto wait_time = 1000;
    while (cv::getWindowProperty(firstAndSecondAdded, cv::WND_PROP_VISIBLE) >= 1) {
        auto keyCode = cv::waitKey(wait_time);
        if (keyCode == 27) { // Wait for ESC key stroke
            cv::destroyAllWindows();
            break;
        }
    }

    cv::destroyAllWindows(); //destroy all open windows
    setText("Addiere Bilder");
    newAlphaAdd = alpha;
}

void AddPicturesCommand::undo()
{
    myPicture->setSecondPath(oldSecondPath);
    myPicture->setAlphaAdd(oldAlphaAdd);
    cv::Mat img1 = cv::imread(myPicture->getCurrPath());
    cv::Mat img2 = cv::imread(myPicture->getSecondPath());
    cv::Mat dst;
    if (img2.empty()){
        dst = img1;
    }
    else {
        dst = OpenCVWrapper::Add(img1, img2, newAlphaAdd);
    }
    QImage qim = OpenCVWrapper::Mat2QImage(dst);
    myPicture->setPixmap(QPixmap::fromImage(qim));
    myPicture->setAlphaAdd(newAlphaAdd);
    myGraphicsScene->update();
}

void AddPicturesCommand::redo()
{
    cv::Mat img1 = cv::imread(myPicture->getCurrPath());

    // using brightness gamma and rgb gammas as predecessor
    img1 = OpenCVWrapper::GammaBrightness(img1, myPicture->getGamma());
    img1 = OpenCVWrapper::ScaleRGB(img1, myPicture->getGammaRed(), myPicture->getGammaGreen(), myPicture->getGammaBlue());

    cv::Mat img2 = cv::imread(myPicture->getSecondPath());

    // using brightness gamma and rgb gammas as predecessor
    img2 = OpenCVWrapper::GammaBrightness(img2, myPicture->getGamma());
    img2 = OpenCVWrapper::ScaleRGB(img2, myPicture->getGammaRed(), myPicture->getGammaGreen(), myPicture->getGammaBlue());

    cv::Mat dst =  OpenCVWrapper::Add(img1, img2, newAlphaAdd);
    QImage qim = OpenCVWrapper::Mat2QImage(dst);
    myPicture->setPixmap(QPixmap::fromImage(qim));
    myPicture->setAlphaAdd(newAlphaAdd);
    myGraphicsScene->update();
}

ChangeTextFontCommand::ChangeTextFontCommand(QString fontNameToSet, int fontSize, QGraphicsTextItem* newItem, QUndoCommand* parent) : QUndoCommand(parent), newFontSize(fontSize), myItem(newItem), newFontName(fontNameToSet)
{
    oldFontName = myItem->font().family();
    oldFontSize = myItem->font().pointSize();
    setText("Neuer Font: " + newFontName);
    newFontSize = (fontSize == 0) ? oldFontSize : fontSize;
}

void ChangeTextFontCommand::undo()
{
    myItem->setFont(QFont(oldFontName, oldFontSize));
}

void ChangeTextFontCommand::redo()
{
    myItem->setFont(QFont(newFontName, newFontSize));
}

ModifyShapColorCommand::ModifyShapColorCommand(QAbstractGraphicsShapeItem* shapeItem, int argb, QGraphicsScene* graphicsScene, QUndoCommand* parent) : item(shapeItem), myGraphicsScene(graphicsScene), QUndoCommand(parent)
{
    oldArgb = shapeItem->brush().color().rgba();
    newArgb = argb;
}

void ModifyShapColorCommand::undo()
{
    item->setBrush(QBrush(QColor(oldArgb)));
    myGraphicsScene->update();
}

void ModifyShapColorCommand::redo()
{
    item->setBrush(QBrush(QColor(newArgb)));
    myGraphicsScene->update();
}
