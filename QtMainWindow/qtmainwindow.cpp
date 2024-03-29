#include <filesystem>
#include <qt5/QtPrintSupport/qprintdialog.h>
#include <qt5/QtPrintSupport/qprintpreviewdialog.h>

#include "qtmainwindow.h"
#include "stdafx.h"
#include "Commands.h"
#include "MyPicture.h"

QtMainWindow::QtMainWindow(QWidget *parent)
    : QMainWindow(parent), undoStack(this), undoView(&undoStack), scene(undoStack, this)
{
    ui.setupUi(this);
    ui.graphicsView->setScene(&scene);
    scene.SetView(ui.graphicsView);
    PrepareGeneralContextMenu();

    PreparePictureContextMenu();
    PrepareUndoRedo();

    PicturePropertyConnections();

    PrepareZooming();

    PrepareTopologyChanges();


    PrepareUpdateUI();
    
    //Associate signal with slot: when m_scene transmits message signal, MainWindow receives signal and executes showMessage slot function
    connect(&scene, SIGNAL(message(QString)), this, SLOT(showMessage(QString)));
    clipboard = QApplication::clipboard();

    // MRU
    separatorAct = ui.menuDatei->addSeparator();
    for (int i = 0; i < maxrecentfile; ++i) {
        recentFiles[i] = new QAction(this);
        recentFiles[i]->setVisible(false);
        connect(recentFiles[i], SIGNAL(triggered()), this, SLOT(OpenRecentFile()));
        ui.menuDatei->addAction(recentFiles[i]);
    }
    UpdateRecentFileActions();

    // Keyboard events et al.
    scene.installEventFilter(this);
}

void QtMainWindow::PrepareUpdateUI()
{
    connect(ui.menuEdit, &QMenu::aboutToShow,
        this, &QtMainWindow::itemMenuAboutToShow);
    connect(ui.menuEdit, &QMenu::aboutToHide,
        this, &QtMainWindow::itemMenuAboutToHide);
}

void QtMainWindow::PreparePictureContextMenu()
{
    ui.graphicsView->pictureContextMenu.addAction(ui.action_Brightnesss);
    ui.graphicsView->pictureContextMenu.addAction(ui.action_RGB_scale);
    ui.graphicsView->pictureContextMenu.addAction(ui.action_AddImage);
    ui.graphicsView->pictureContextMenu.addAction(ui.action_Sobel);
    ui.graphicsView->pictureContextMenu.addAction(ui.action_HistogramEqualize);
    ui.graphicsView->pictureContextMenu.addAction(ui.action_SubPicture);
}

void QtMainWindow::PrepareGeneralContextMenu()
{
    ui.graphicsView->contextMenu.addAction(ui.action_Rect);
    ui.graphicsView->contextMenu.addAction(ui.action_Picture);
    ui.graphicsView->contextMenu.addAction(ui.action_Textfield);
    ui.graphicsView->contextMenu.addAction(ui.action_Farbe);
    ui.graphicsView->contextMenu.addAction(ui.action_Font);
}

void QtMainWindow::PrepareUndoRedo()
{
    undoView.setWindowTitle(tr("Kommando Liste"));
    undoView.show();
    undoView.move(0, 0);
    move(undoView.size().width(), 0);
    undoView.setAttribute(Qt::WA_QuitOnClose, false);
    QIcon icon;
    icon.addFile(QString::fromUtf8(":/QtMainWindow/images/ok.png"), QSize(), QIcon::Normal, QIcon::Off);
    undoView.setWindowIcon(icon);

    QAction* undoAction = undoStack.createUndoAction(this);
    QAction* redoAction = undoStack.createRedoAction(this);
    auto seqs = QList< QKeySequence>{ QKeySequence::Undo, Qt::ALT + Qt::Key_Backspace };
    undoAction->setShortcuts(seqs); //Ctrl+Z, Alt+Backspace:setShortcuts() function for shortcut keys
    redoAction->setShortcut(QKeySequence::Redo); //QKeySequence of Ctrl+Y, Shift+Ctrl+Z:Qt defines many built-in shortcut keys for us
    undoAction->setText(tr("&Zur\303\274ck"));
    redoAction->setText(tr("&Wiederholen"));
    undoAction->setToolTip("Mache letzte Aktion weg");
    redoAction->setToolTip("Mache letzte Aktion wieder hin");
    QIcon icon11;
    icon11.addFile(QString::fromUtf8(":/QtMainWindow/images/undo.png"), QSize(), QIcon::Normal, QIcon::Off);
    undoAction->setIcon(icon11);
    QIcon icon12;
    icon12.addFile(QString::fromUtf8(":/QtMainWindow/images/redo.png"), QSize(), QIcon::Normal, QIcon::Off);
    redoAction->setIcon(icon12);
    ui.mainToolBar->addAction(undoAction);
    ui.mainToolBar->addAction(redoAction);
    ui.menuEdit->addAction(undoAction);
    ui.menuEdit->addAction(redoAction);  //Add two actions to edit
    connect(&undoStack, SIGNAL(indexChanged(int)), this, SLOT(undoIndexChanged(int)));
}

void QtMainWindow::PrepareTopologyChanges()
{
    connect(&scene, &MyScene::itemMoved,
        this, &QtMainWindow::itemMoved);
    connect(ui.graphicsView, &MyGraphicsView::itemScaled,
        this, &QtMainWindow::itemScaled);
    connect(ui.graphicsView, &MyGraphicsView::itemRotated,
        this, &QtMainWindow::itemRotated);
    connect(&scene, &MyScene::selectionChanged,
        this, &QtMainWindow::sceneSelectionChanged);
}

void QtMainWindow::PrepareZooming()
{
    // Add values in the combo box for zoom
    myZoomComboBox.addItem("1.0");
    myZoomComboBox.addItem("0.5");
    myZoomComboBox.addItem("1.5");
    myZoomComboBox.setEditable(true);
    myZoomComboBox.setStatusTip("Set zoom factor for scene");
    myZoomComboBox.setToolTip("Zoom factor for scene");
    myZoomComboBox.setMinimumContentsLength(10);
    ui.toolBar->addWidget(&myZoomComboBox);
    // make the connection between the combo box and a slot
    connect(&myZoomComboBox, SIGNAL(currentIndexChanged(int)),
        SLOT(zoomComboIndexChanged(int)));
    connect(&myZoomComboBox, SIGNAL(activated(const QString&)),
        SLOT(zoomComboStringChanged(const QString&)));
    connect(ui.graphicsView, &MyGraphicsView::zoomed,
        this, &QtMainWindow::zoomed);
}

void QtMainWindow::PicturePropertyConnections()
{
    // Properties
    connect(ui.ScaleFactor, SIGNAL(valueChanged(double)), this, SLOT(setSelScale(double)));
    connect(ui.Rotation, SIGNAL(valueChanged(double)), this, SLOT(setSelRotation(double)));
    connect(ui.Brightness, SIGNAL(valueChanged(double)), this, SLOT(setSelGamma(double)));
    connect(ui.dial_Red, SIGNAL(valueChanged(int)), this, SLOT(setSelGammaRed(int)));
    connect(ui.dial_Green, SIGNAL(valueChanged(int)), this, SLOT(setSelGammaGreen(int)));
    connect(ui.dial_Blue, SIGNAL(valueChanged(int)), this, SLOT(setSelGammaBlue(int)));
    connect(ui.ZValue, SIGNAL(valueChanged(double)), this, SLOT(setSelZvalue(double)));
    connect(ui.fontComboBox, SIGNAL(activated(const QString&)), this, SLOT(setSelFont(double)));
    connect(ui.fontComboBox, SIGNAL(currentIndexChanged(int)), SLOT(fontComboIndexChanged(int)));
}

void QtMainWindow::showMessage(QString msg)
{
    statusBar()->showMessage(msg);  // Display messages on the main window status bar
}

bool QtMainWindow::checkSelection(int num)
{
    if (scene.selectedItems().isEmpty())
        return false;
    if (scene.selectedItems().count() != num) {
        QMessageBox::warning(this, "Applikation",
            "Die Anzahl selektierter Elemente passt nicht",
            QMessageBox::Discard);
        return false;
    }
    return true;
}

void QtMainWindow::UpdateRecentFileActions()
{
    QSettings settings("DsQt", "PictureManager");
    QStringList files = settings.value("recentFileList").toStringList();
    int numRecentFiles = qMin(files.size(), maxrecentfile);

    for (int i = 0; i < numRecentFiles; ++i) {
        QString text = tr("&%1 %2").arg(i + 1).arg(files[i].trimmed());
        recentFiles[i]->setText(text);
        recentFiles[i]->setData(files[i]);
        recentFiles[i]->setVisible(true);
    }
    for (int j = numRecentFiles; j < maxrecentfile; ++j) {
        recentFiles[j]->setVisible(false);
    }
    separatorAct->setVisible(numRecentFiles > 0);
}

void QtMainWindow::OpenRecentFile()
{
    QAction* action = qobject_cast<QAction*>(sender());
    if (action) {
        if (maybeSave()) {
            scene.New();
            undoStack.clear();
            QString filename = action->data().toString();
            if (filename.isEmpty())
                return;
            std::filesystem::path p = filename.toStdString();
            currdir = QString::fromStdString(p.parent_path().string());
            scene.load(filename);
            setCurrentFile(filename);
        }
    }
}

void QtMainWindow::setCurrentFile(const QString& fileName)
{
    setWindowFilePath(fileName);

    QSettings settings("DsQt", "PictureManager");
    QStringList files = settings.value("recentFileList").toStringList();
    files.removeAll(fileName);
    files.prepend(fileName);
    while (files.size() > maxrecentfile)
        files.removeLast();

    settings.setValue("recentFileList", files);

    UpdateRecentFileActions();
}

void QtMainWindow::on_action_New_triggered()
{
    if (maybeSave()) {
        undoStack.clear();
        scene.New();
    }
}

void QtMainWindow::itemMoved(QList<QGraphicsItem*>& movedItems, std::vector<QPointF>& moveStartPositions)
{
    undoStack.push(new MoveCommand(movedItems, moveStartPositions));
}

void QtMainWindow::itemScaled(QGraphicsItem* item, double oldScale)
{
    undoStack.push(new ScaleCommand(item, oldScale));
}

void QtMainWindow::itemRotated(QGraphicsItem* item, double oldRotation)
{
    undoStack.push(new RotateCommand(item, oldRotation));
}

bool QtMainWindow::eventFilter(QObject* watched, QEvent* event)
{
    switch (event->type()) {
        case QEvent::GraphicsSceneMouseMove:
        {
            QGraphicsSceneMouseEvent* mouseEvent = dynamic_cast<QGraphicsSceneMouseEvent*>(event);
            ui.graphicsView->cursor().setShape(Qt::CrossCursor);
            cursor().setShape(Qt::CrossCursor);
            qDebug() << mouseEvent->type();
            qDebug() << "x: " << mouseEvent->scenePos().x();
            qDebug() << "y: " << mouseEvent->scenePos().y();
            if (scene.itemAt(mouseEvent->scenePos().x(), mouseEvent->scenePos().y(), QTransform()) != nullptr) {
                setCursor(QCursor(Qt::ArrowCursor));
            }
            else {
                setCursor(QCursor(Qt::CrossCursor));
            }
        }
        break;
        case QEvent::GraphicsSceneHoverMove:
        case QEvent::GraphicsSceneHoverEnter:
        case QEvent::GraphicsSceneHoverLeave:
        {
            QGraphicsSceneHoverEvent* mouseEvent = dynamic_cast<QGraphicsSceneHoverEvent*>(event);
            qDebug() << mouseEvent->type();
            qDebug() << mouseEvent->widget();
        }
        break;
        case QEvent::GraphicsSceneContextMenu:
        break;
        case QEvent::Leave:
        {
            setCursor(QCursor(Qt::ArrowCursor));
        }
        break;
        case QEvent::GraphicsSceneMousePress:
        {
            QGraphicsSceneMouseEvent* mouseEvent = dynamic_cast<QGraphicsSceneMouseEvent*>(event);
        }
        break;
        case QEvent::GraphicsSceneMouseRelease:
        {
        }
        break;
        default:
        {
            qDebug() << event->type();
        }
    }
    auto* focusItem = scene.focusItem();
    if (focusItem == nullptr || focusItem->type() != QGraphicsTextItem::Type)
        return false;
    auto* doc = static_cast<QGraphicsTextItem*>(focusItem)->document();

    if (event->type() != QEvent::KeyPress)
        return false;
    QKeyEvent* key_event = dynamic_cast<QKeyEvent*>(event);
    QKeySequence key_sequence{ static_cast<int>(key_event->modifiers()) + key_event->key() };

    if (key_sequence == QKeySequence::Undo)
    {
        undoStack.undo(); // nothing in the stack
        qDebug() << "undo";
        return true;
    }
    else if (key_sequence == QKeySequence::Redo)
    {
        undoStack.redo(); // nothing in the stack
        qDebug() << "redo";
        return true;
    }
    return false;
}

void QtMainWindow::closeEvent(QCloseEvent* event)
{
    if (maybeSave()) {
        event->accept();
    }
    else {
        event->ignore();
    }
}

bool QtMainWindow::maybeSave()
{
    if (!scene.isModified()) {
        return true;
    }
    const QMessageBox::StandardButton ret
        = QMessageBox::warning(this, "Applikation",
            "Die Szene wurde modifiziert.\n"
                "Sollen die Anpassungen gespeichert werden?",
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    switch (ret) {
    case QMessageBox::Save:
        on_action_Save_triggered();
        return false;
    case QMessageBox::Cancel:
        return false;
    default:
        break;
    }
    return true;
}

void QtMainWindow::on_action_Open_triggered()
{
    if (maybeSave()) {
        scene.New();
        undoStack.clear();
        QString filename = QFileDialog::getOpenFileName(this, "Lade Szene", currdir, "Json File (*.json)");
        if (filename.isEmpty())
            return;
        std::filesystem::path p = filename.toStdString();
        currdir = QString::fromStdString(p.parent_path().string());
        scene.load(filename);
        setCurrentFile(filename);
    }
}

void QtMainWindow::on_action_Save_triggered()
{
    if (!scene.save()) {
        on_action_SaveAs_triggered();
    }
}

void QtMainWindow::on_action_SaveAs_triggered()
{
    QString filename = QFileDialog::getSaveFileName(this, "Speichern unter:", currdir, "Json File (*.json)");
    if (filename.isEmpty())
        return;
    std::filesystem::path p = filename.toStdString();
    currdir = QString::fromStdString(p.parent_path().string());
    scene.save(filename);
    setCurrentFile(filename);
}

void QtMainWindow::on_action_Copy_triggered()
{
    if (!scene.selectedItems().empty()) {
        QJsonObject jsonSelection = scene.toJson(scene.selectedItems());

        clipboard->setText(QJsonDocument(jsonSelection).toJson().data());
    }
}

void QtMainWindow::on_action_Cut_triggered()
{
    if (!scene.selectedItems().empty()) {
        on_action_Copy_triggered();
        on_action_Delete_triggered();
        scene.SetModified(true);
    }
}

void QtMainWindow::on_action_Paste_triggered()
{
    QString originalText = clipboard->text();
    if (!originalText.isEmpty()) {
        scene.SetModified(true);
        undoStack.push(new AddPasteCommand(clipboard->text(), &scene));
    }
}

void QtMainWindow::on_action_Rect_triggered()
{
    QBrush greenBrush(Qt::green);
    QBrush blueBrush(Qt::blue);
    QPen outlinePen(Qt::black);
    outlinePen.setWidth(2);
    QPoint pos(ui.graphicsView->cursor().pos());
    pos = ui.graphicsView->mapToScene(ui.graphicsView->mapFromGlobal(pos)).toPoint();

    QGraphicsRectItem* rectangle = new QGraphicsRectItem(100, 0, 80, 100);
    rectangle->setBrush(greenBrush);
    rectangle->setPen(outlinePen);
    rectangle->setFlag(QGraphicsItem::ItemIsMovable);
    rectangle->setFlag(QGraphicsItem::ItemIsSelectable);
    undoStack.push(new AddBoxCommand(rectangle, &scene, pos));

    QGraphicsRectItem* rect = new QGraphicsRectItem(0, 0, 100, 100);
    rect->setBrush(blueBrush);
    rect->setPen(outlinePen);
    rect->setFlag(QGraphicsItem::ItemIsMovable);
    rect->setFlag(QGraphicsItem::ItemIsSelectable);
    scene.SetModified(true);
    undoStack.push(new AddBoxCommand(rect, &scene, pos));
}

void QtMainWindow::on_action_Picture_triggered()
{
    QString filename = QFileDialog::getOpenFileName(this, "Hole Bild", currdir, "All picture Files (*.jpg *.jpeg *.png *.tiff *.bmp)");
    if (filename.isEmpty())
        return;
    std::filesystem::path p = filename.toStdString();
    currdir = QString::fromStdString(p.parent_path().string());
    undoStack.push(new AddPictureCommand(p.generic_string(), &scene));
    scene.SetModified(true);
}

void QtMainWindow::on_action_Textfield_triggered()
{
    QGraphicsTextItem* text = new QGraphicsTextItem("Ersetzen");
    auto* doc = text->document();
    connect(doc, SIGNAL(undoCommandAdded()), this, SLOT(textChange()));
    text->setFont(QFont("Arial", 20));
    text->setFlag(QGraphicsItem::ItemIsMovable);
    text->setFlag(QGraphicsItem::ItemIsSelectable);
    text->setFlag(QGraphicsItem::ItemIsPanel);
    text->setTextInteractionFlags(Qt::TextInteractionFlag::TextEditorInteraction);
    QPoint pos(ui.graphicsView->cursor().pos());
    qDebug() << pos;
    pos = ui.graphicsView->mapToScene(ui.graphicsView->mapFromGlobal( pos)).toPoint();
    qDebug() << pos;
    undoStack.push(new AddBoxCommand(text, &scene, pos));
    scene.SetModified(true);
}

void QtMainWindow::on_action_ZoomIn_triggered()
{
    ui.graphicsView->zoomIn();
}

void QtMainWindow::on_action_ZoomOut_triggered()
{
    ui.graphicsView->zoomOut();
}

void QtMainWindow::on_action_Delete_triggered()
{
    if (scene.selectedItems().isEmpty())
        return;

    QUndoCommand* deleteCommand = new DeleteCommand(&scene);
    undoStack.push(deleteCommand);
}

void QtMainWindow::on_action_Brightnesss_triggered()
{
    if (!checkSelection(1))
        return;
    QGraphicsItem* item = scene.selectedItems().first();
    if (item != nullptr && item->type() == QGraphicsPixmapItem::Type) {
        hide();
        ModifyBrightnessCommand* modCommand = new ModifyBrightnessCommand(static_cast<MyPicture*>(item), &scene);
        undoStack.push(modCommand);
        show();
    }
}

void QtMainWindow::on_action_RGB_scale_triggered()
{
    if (!checkSelection(1))
        return;
    QGraphicsItem* item = scene.selectedItems().first();
    if (item != nullptr && item->type() == QGraphicsPixmapItem::Type) {
        hide();
        ModifyRGBScaleCommand* modCommand = new ModifyRGBScaleCommand(static_cast<MyPicture*>(item), &scene);
        undoStack.push(modCommand);
        show();
    }
}

void QtMainWindow::on_action_AddImage_triggered()
{
    if (scene.selectedItems().count() != 2 ||
        scene.selectedItems().count() == 2 && (scene.selectedItems()[0]->type() != QGraphicsPixmapItem::Type || scene.selectedItems()[1]->type() != QGraphicsPixmapItem::Type)) {
        QMessageBox::warning(this, "Applikation",
            reinterpret_cast<const char*>(u8"Es sollten zwei Bilder ausgewählt sein!"),
            QMessageBox::Discard);
        return;
    }
    auto* img1 = static_cast<MyPicture*>(scene.selectedItems()[0]);
    auto* img2 = static_cast<MyPicture*>(scene.selectedItems()[1]);

    if (img1->boundingRect().width() / img1->boundingRect().height() != img2->boundingRect().width() / img2->boundingRect().height()) {
        QMessageBox::warning(this, "Applikation",
            reinterpret_cast<const char*>(u8"Es sollten zwei Bilder mit gleichem Seitenverhältnis sein!"),
            QMessageBox::Discard);
        return;
    }
    hide();
    undoStack.push(new AddPicturesCommand(img1, img2, &scene));
    show();
}

void QtMainWindow::on_action_Sobel_triggered()
{
    if (!checkSelection(1))
        return;
    QGraphicsItem* item = scene.selectedItems().first();
    if (item != nullptr && item->type() == QGraphicsPixmapItem::Type) {
        auto* mPic = static_cast<MyPicture*>(item);
        cv::Mat orgImg = cv::imread(mPic->getCurrPath());
        hide();
        cv::Mat res = lSobel.Sobel(orgImg);
        auto sobelPath = QString::fromStdString(mPic->getCurrPath()).replace(QRegExp(R"(\.(\w+))"), R"(_Sobel.\1)");
        res.convertTo(res, CV_8UC4, 255.0);
        cv::imwrite(sobelPath.toStdString(), res);
        undoStack.push(new AddPictureCommand(sobelPath.toStdString(), &scene));
        show();
    }
    else {
        QMessageBox::warning(this, "Applikation",
            reinterpret_cast<const char*>(u8"Sobel geht nur auf einem Bild!"),
            QMessageBox::Discard);
    }
}

void QtMainWindow::on_action_HistogramEqualize_triggered()
{
    if (!checkSelection(1))
        return;
    QGraphicsItem* item = scene.selectedItems().first();
    if (item != nullptr && item->type() == QGraphicsPixmapItem::Type) {
        auto* mPic = static_cast<MyPicture*>(item);
        cv::Mat orgImg = cv::imread(mPic->getCurrPath());
        hide();
        cv::Mat res = OpenCVWrapper::ColorHistEqualization(orgImg);
        auto histPath = QString::fromStdString(mPic->getCurrPath()).replace(QRegExp(R"(\.(\w+))"), R"(_Hist.\1)");
        cv::imwrite(histPath.toStdString(), res);
        undoStack.push(new AddPictureCommand(histPath.toStdString(), &scene));
        show();
    }
    else {
        QMessageBox::warning(this, "Applikation",
            reinterpret_cast<const char*>(u8"Histogramm Abgleich geht nur auf einem Bild!"),
            QMessageBox::Discard);
    }
}

void QtMainWindow::on_action_Farbe_triggered()
{
    if (!checkSelection(1))
        return;
    QGraphicsItem* item = scene.selectedItems().first();
    if (item != nullptr && item->type() == QGraphicsRectItem::Type) {
        auto* mRect = static_cast<QGraphicsRectItem*>(item);
        
        QColor newColor = QColorDialog::getColor(mRect->brush().color(), this, "Neue Objektfarbe", QColorDialog::ColorDialogOption::ShowAlphaChannel);
        undoStack.push(new ModifyShapColorCommand(mRect, newColor.rgba(), &scene));
    }
}

void QtMainWindow::on_action_SubPicture_triggered()
{
    if (!checkSelection(1))
        return;
    QGraphicsItem* item = scene.selectedItems().first();
    if (item != nullptr && item->type() == QGraphicsPixmapItem::Type) {
        auto* mPic = static_cast<MyPicture*>(item);
        hide();
        auto partPath = OpenCVWrapper::saveSubPicture(mPic->getCurrPath());
        if (partPath.empty()) {
            QMessageBox::warning(this, "Applikation",
                reinterpret_cast<const char*>(u8"Teilbild wird nicht gespeichert!"),
                QMessageBox::Discard);
            show();
            return;
        }
        undoStack.push(new AddPictureCommand(partPath, &scene));
        show();
    }
    else {
        QMessageBox::warning(this, "Applikation",
            reinterpret_cast<const char*>(u8"Teilbild geht nur auf einem Bild!"),
            QMessageBox::Discard);
    }
}

void QtMainWindow::on_action_Print_triggered()
{
    QPrinter printer(QPrinter::HighResolution);
    QPrintDialog dialog(&printer, this);
    dialog.setWindowTitle(tr("Print Document"));
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }
    auto name = printer.printerName();
    if (name.contains("PDF|PS")) {
        QString filename = QFileDialog::getSaveFileName(this, "Drucke Szene in Datei:", currdir, "Pdf File (*.pdf)");
        printer.setOutputFileName(filename);
    }
    print(&printer);
}

void QtMainWindow::on_action_Print_Preview_triggered()
{
    QPrinter printerPrev(QPrinter::HighResolution);
    QPrintPreviewDialog printPreview(&printerPrev, this);
    connect(&printPreview, SIGNAL(paintRequested(QPrinter*)), this, SLOT(print(QPrinter*)));
    printPreview.exec();
}

void QtMainWindow::on_action_Font_triggered()
{
    if (!checkSelection(1))
        return;
    QGraphicsItem* item = scene.selectedItems().first();
    if (item != nullptr && item->type() == QGraphicsTextItem::Type) {
        auto* mText = static_cast<QGraphicsTextItem*>(item);

        bool ok{false};
        QFont font = QFontDialog::getFont(
            &ok, mText->font(), this, "Font des Texts");
        undoStack.push(new ChangeTextFontCommand(font.family(), font.pointSize(), mText));
        scene.SetModified(true);
    }
}

void QtMainWindow::print(QPrinter* printer)
{
    QPainter painter(printer);
    scene.render(&painter);
}

void QtMainWindow::itemMenuAboutToShow()
{
    ui.action_Delete->setEnabled(!scene.selectedItems().isEmpty());
}

void QtMainWindow::itemMenuAboutToHide()
{
    ui.action_Delete->setEnabled(true);
}

void QtMainWindow::zoomComboIndexChanged(int index)
{
    auto sel = myZoomComboBox.itemText(index);
    zoomComboStringChanged(sel);
}

void QtMainWindow::zoomComboStringChanged(const QString& sel)
{
    try {
        double zoomFactor = std::stod(sel.toStdString());
        ui.graphicsView->resetTransform();
        ui.graphicsView->scale(zoomFactor, zoomFactor);
    }
    catch (...) {
        QMessageBox::warning(this, "Applikation",
            reinterpret_cast<const char*>(u8"Es sollte ein vernünftiger real Wert sein!"),
            QMessageBox::Discard);
    }
}

void QtMainWindow::zoomed()
{
    auto trf = ui.graphicsView->transform();
    double scale = sqrt(trf.m11() * trf.m11() + trf.m12() * trf.m12());
    std::string s = std::to_string(scale);
    myZoomComboBox.setCurrentText(QString::fromStdString(s));
}

void QtMainWindow::fontComboIndexChanged(int index)
{
    auto sel = ui.fontComboBox->itemText(index);
    setSelFont(sel);
}

void QtMainWindow::textChange()
{
    auto* focused = scene.focusItem();
    if (focused->type() == QGraphicsTextItem::Type) {
        auto* myText = static_cast<QGraphicsTextItem*>(focused);
        auto cmd = std::make_unique<ChangeTextCommand>(myText);
        undoStack.push(cmd.release());
    }
}

void QtMainWindow::setSelectedLabelText(const MyPicture* pic)
{
    ui.SelectedObject->setText(QString::fromStdString(pic->getCurrPath()));
}

void QtMainWindow::setSelectedLabelText(const QGraphicsTextItem* txt)
{
    ui.SelectedObject->setText(txt->toPlainText());
}

void QtMainWindow::setSelectedLabelText(const QGraphicsRectItem* rect)
{
    ui.SelectedObject->setText("Rectangular shape");
}

void QtMainWindow::undoIndexChanged(int idx)
{
    // Probably influence on the selection's properties
    sceneSelectionChanged();
}

template <typename T>
    requires std::is_base_of_v<QGraphicsItem, T>
void QtMainWindow::fillProps(T* sel)
{
    setSelectedLabelText(sel);
    ui.ScaleFactor->setValue(sel->scale());
    ui.Rotation->setValue(sel->rotation());
    if constexpr (std::is_same_v<T, MyPicture>) {
        ui.Brightness->setValue(sel->getGamma());
        ui.dial_Red->setValue(sel->getGammaRed() * 100);
        ui.dial_Green->setValue(sel->getGammaGreen() * 100);
        ui.dial_Blue->setValue(sel->getGammaBlue() * 100);
    }
    else {
        ui.Brightness->setValue(1.0);
    }
    ui.ZValue->setValue(sel->zValue());
    if constexpr (std::is_same_v<T, QGraphicsTextItem>) {
        ui.fontComboBox->setCurrentFont(sel->font());
    }
    else {
        ui.fontComboBox->setCurrentFont(QFont());
    }
}

void QtMainWindow::sceneSelectionChanged()
{
    if (scene.selectedItems().count() != 1) {
        ui.SelectedObject->setText("Non unique selection!");
    }
    else {
        auto* sel = scene.selectedItems().first();
        auto selType = sel->type();
        switch (selType)
        {
        case QGraphicsRectItem::Type:
            {
                auto* rect = static_cast<QGraphicsRectItem*>(sel);
                fillProps(rect);
        }
            break;
        case QGraphicsTextItem::Type:
            {
                auto* text = static_cast<QGraphicsTextItem*>(sel);
                fillProps(text);
            }
            break;
        case QGraphicsPixmapItem::Type:
            {
                auto* pic = static_cast<MyPicture*>(sel);
                fillProps(pic);
            }
            break;
        default:
            break;
        }
    }
}

void QtMainWindow::setSelScale(double newScale)
{
    if (scene.selectedItems().count() != 1)
        return;
    auto* sceneItem = scene.selectedItems().first();
    auto currScale = sceneItem->scale();
    if (newScale == currScale)
        return; // avoid event circle!
    sceneItem->setScale(newScale);
    itemScaled(sceneItem, currScale);
}

void QtMainWindow::setSelRotation(double newScale)
{
    if (scene.selectedItems().count() != 1)
        return;
    auto* sceneItem = scene.selectedItems().first();
    auto currRotation = sceneItem->rotation();
    if (newScale == currRotation)
        return; // avoid event circle!
    sceneItem->setRotation(newScale);
    itemRotated(sceneItem, currRotation);
}

void QtMainWindow::setSelGamma(double newGamma)
{
    if (scene.selectedItems().count() != 1)
        return;
    auto* sceneItem = scene.selectedItems().first();
    if (sceneItem->type() != QGraphicsPixmapItem::Type)
        return; // Only relevant for pictures
    auto* myPic = static_cast<MyPicture*>(sceneItem);
    auto oldGamma = myPic->getGamma();
    if (oldGamma == newGamma)
        return; // avoid event circle!
    undoStack.push(new SetBrightnessCommand(newGamma, myPic, &scene));
}

void QtMainWindow::setSelGammaRed(int newGammaRed)
{
    if (scene.selectedItems().count() != 1)
        return;
    auto* sceneItem = scene.selectedItems().first();
    if (sceneItem->type() != QGraphicsPixmapItem::Type)
        return; // Only relevant for pictures
    auto* myPic = static_cast<MyPicture*>(sceneItem);
    auto oldGammaRed = myPic->getGammaRed();
    if (oldGammaRed == newGammaRed / 100.0)
        return; // avoid event circle!
    undoStack.push(new SetRGBScaleCommand(newGammaRed / 100.0, myPic->getGammaGreen(), myPic->getGammaBlue(), myPic, &scene));
    auto ga = MyPicfoo(*myPic);
    qDebug() << ga;
}

void QtMainWindow::setSelGammaGreen(int newGammaGreen)
{
    if (scene.selectedItems().count() != 1)
        return;
    auto* sceneItem = scene.selectedItems().first();
    if (sceneItem->type() != QGraphicsPixmapItem::Type)
        return; // Only relevant for pictures
    auto* myPic = static_cast<MyPicture*>(sceneItem);
    auto oldGammaGreen = myPic->getGammaGreen();
    if (oldGammaGreen == newGammaGreen / 100.0)
        return; // avoid event circle!
    undoStack.push(new SetRGBScaleCommand(myPic->getGammaRed(), newGammaGreen / 100.0, myPic->getGammaBlue(), myPic, &scene));
}

void QtMainWindow::setSelGammaBlue(int newGammaBlue)
{
    if (scene.selectedItems().count() != 1)
        return;
    auto* sceneItem = scene.selectedItems().first();
    if (sceneItem->type() != QGraphicsPixmapItem::Type)
        return; // Only relevant for pictures
    auto* myPic = static_cast<MyPicture*>(sceneItem);
    auto oldGammaBlue = myPic->getGammaBlue();
    if (oldGammaBlue == newGammaBlue / 100.0)
        return; // avoid event circle!
    undoStack.push(new SetRGBScaleCommand(myPic->getGammaRed(), myPic->getGammaGreen(), newGammaBlue / 100.0, myPic, &scene));
}

void QtMainWindow::setSelZvalue(double newZvalue)
{
    if (scene.selectedItems().count() != 1)
        return;
    auto* sceneItem = scene.selectedItems().first();
    undoStack.push(new ZvalueCommand(sceneItem, newZvalue));
}

void QtMainWindow::setSelFont(QString fontName)
{
    if (scene.selectedItems().count() != 1)
        return;
    auto* sceneItem = scene.selectedItems().first();
    if (sceneItem->type() != QGraphicsTextItem::Type)
        return; // Only relevant for texts
    auto* myText = static_cast<QGraphicsTextItem*>(sceneItem);
    const auto& currFont = myText->font().family();
    if (currFont == fontName)
        return; // avoid event circle!
    undoStack.push(new ChangeTextFontCommand(fontName, 0, myText));
}
