#include <opencv2/opencv.hpp>
#include <filesystem>

#include "qtmainwindow.h"
#include "stdafx.h"
#include "OpenCVWrapper.h"
#include "Commands.h"
#include "MyPicture.h"

QtMainWindow::QtMainWindow(QWidget *parent)
    : QMainWindow(parent), undoStack(this), undoView(&undoStack), scene(undoStack, this)
{
    ui.setupUi(this);
    ui.graphicsView->setScene(&scene);
    ui.graphicsView->contextMenu.addAction(ui.action_Rect);
    ui.graphicsView->contextMenu.addAction(ui.action_Picture);
    ui.graphicsView->contextMenu.addAction(ui.action_Textfield);

    ui.graphicsView->pictureContextMenu.addAction(ui.action_Brightnesss);
    ui.graphicsView->pictureContextMenu.addAction(ui.action_RGB_scale);
    undoView.setWindowTitle(tr("Kommando Liste"));
    undoView.show();
    undoView.setAttribute(Qt::WA_QuitOnClose, false);

    QAction* undoAction = undoStack.createUndoAction(this);
    QAction* redoAction = undoStack.createRedoAction(this);
    auto seqs = QList< QKeySequence>{ QKeySequence::Undo, Qt::ALT + Qt::Key_Backspace};
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
    connect(undoAction, SIGNAL(triggered(bool)), this, SLOT(undone(bool)));
    connect(redoAction, SIGNAL(triggered(bool)), this, SLOT(redone(bool)));

    connect(&scene, &MyScene::itemMoved,
        this, &QtMainWindow::itemMoved);

    connect(ui.menuEdit, &QMenu::aboutToShow,
        this, &QtMainWindow::itemMenuAboutToShow);
    connect(ui.menuEdit, &QMenu::aboutToHide,
        this, &QtMainWindow::itemMenuAboutToHide);
    
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

    // Keyboard events
    scene.installEventFilter(this);
}

void QtMainWindow::showMessage(QString msg)
{
    statusBar()->showMessage(msg);  // Display messages on the main window status bar
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

void QtMainWindow::itemMoved(QGraphicsItem* movedItem, const QPointF& moveStartPosition)
{
    undoStack.push(new MoveCommand(movedItem, moveStartPosition));
}

bool QtMainWindow::eventFilter(QObject* watched, QEvent* event)
{
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
    QString filename = QFileDialog::getSaveFileName(this, "Hole Bild", currdir, "Json File (*.json)");
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

    QGraphicsRectItem* rectangle = new QGraphicsRectItem(100, 0, 80, 100);
    rectangle->setBrush(greenBrush);
    rectangle->setPen(outlinePen);
    rectangle->setFlag(QGraphicsItem::ItemIsMovable);
    rectangle->setFlag(QGraphicsItem::ItemIsSelectable);
    undoStack.push(new AddBoxCommand(rectangle, &scene));

    QGraphicsRectItem* rect = new QGraphicsRectItem(0, 0, 100, 100);
    rect->setBrush(blueBrush);
    rect->setPen(outlinePen);
    rect->setFlag(QGraphicsItem::ItemIsMovable);
    rect->setFlag(QGraphicsItem::ItemIsSelectable);
    undoStack.push(new AddBoxCommand(rect, &scene));
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
    text->setTextInteractionFlags(Qt::TextInteractionFlag::TextEditorInteraction);
    undoStack.push(new AddBoxCommand(text, &scene));
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
    if (scene.selectedItems().isEmpty())
        return;
    QGraphicsItem* item = scene.selectedItems().first();
    if (item != nullptr && item->type() == QGraphicsPixmapItem::Type) {
        ModifyBrightnessCommand* modCommand = new ModifyBrightnessCommand(static_cast<MyPicture*>(item), &scene);
        undoStack.push(modCommand);
    }
}

void QtMainWindow::on_action_RGB_scale_triggered()
{
    if (scene.selectedItems().isEmpty())
        return;
    QGraphicsItem* item = scene.selectedItems().first();
    if (item != nullptr && item->type() == QGraphicsPixmapItem::Type) {
        ModifyRGBScaleCommand* modCommand = new ModifyRGBScaleCommand(static_cast<MyPicture*>(item), &scene);
        undoStack.push(modCommand);
    }
}

void QtMainWindow::itemMenuAboutToShow()
{
    ui.action_Delete->setEnabled(!scene.selectedItems().isEmpty());
}

void QtMainWindow::itemMenuAboutToHide()
{
    ui.action_Delete->setEnabled(true);
}

void QtMainWindow::undone(bool checked)
{
}

void QtMainWindow::redone(bool checked)
{
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
