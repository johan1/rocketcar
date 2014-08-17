#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "newleveldialog.h"
#include "config.h"

#include <QFileDialog>
#include <QGraphicsItem>
#include <QMouseEvent>
#include <QWidget>

// For logging.
#include <iostream>
#include <cmath>

MainWindow::MainWindow(QWidget *parent) :
        QMainWindow(parent),
        ui(new Ui::MainWindow),
		scale(64.0f),
        selectedGameObject(nullptr) {
    ui->setupUi(this);
    setupToolbar();

    updateTitle();
    setupEditorGraphics();
}

void MainWindow::setupToolbar() {
    // QMenu* menu
    for (auto const& category : Config::getInstance().getCategories()) {
        auto menuPtr = new QMenu(QString::fromStdString(category.name));
        gameObjectGroups.emplace_back(menuPtr);
        menuPtr->menuAction()->setIcon(QIcon(
                QString::fromStdString(Config::getInstance().getAssetsPath() + "/" + category.iconPath)));

        for (auto &gameObject : Config::getInstance().getGameObjects()) {
            if (gameObject.category == category.name) {
                auto goAction = menuPtr->addAction(QIcon(QString::fromStdString(Config::getInstance().getAssetsPath() + "/" + gameObject.imagePath)),
                                   QString::fromStdString(gameObject.name));
                void* voidData = static_cast<void*>(const_cast<GameObject*>(&gameObject)); // This is some horrible shit!
                goAction->setData(qVariantFromValue(voidData));
                connect(goAction, SIGNAL(triggered()), this, SLOT(gameObjectPicked()));
            }
        }

        ui->toolBar->addAction(menuPtr->menuAction());
    }
}

// TODO: When we introduce shortcuts for picking the game object we might need to change this...
void MainWindow::gameObjectPicked() {
    auto goAction = dynamic_cast<QAction*>(sender());
    selectedGameObject = const_cast<GameObject const*>(static_cast<GameObject*>(goAction->data().value<void*>())); // Follows from horrible shit!

    // Mark as selected.
    static_cast<QMenu*>(goAction->parentWidget())->menuAction()->setIcon(
            QIcon(QString::fromStdString(Config::getInstance().getAssetsPath() + "/" + selectedGameObject->imagePath)));

	ui->graphicsView->setCurrentGameObject(selectedGameObject);
}

void MainWindow::exit() {
    qApp->exit();
}

void MainWindow::newLevel() {
    NewLevelDialog nld;
    if (nld.exec()) {
        newLevel(std::move(nld.getEditor()));
    }
}

void MainWindow::newLevel(LevelEditor && editor) {
    levelEditor = std::move(editor);
    updateTitle();
    setupEditorGraphics();
}

void MainWindow::openLevel() {
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Level"), ".", tr("Text files (*.json)"));
    filePath = fileName.toStdString();
    if (!filePath.empty()) {
        levelEditor = LevelEditor(filePath);
        updateTitle();
        setupEditorGraphics();
    }
}

void MainWindow::saveLevel() {
    bool fileSelected = false;
    if (filePath.empty()) {
        QString fileName = QFileDialog::getSaveFileName(this, tr("Save Level"), ".", tr("Text files (*.json)"));
        filePath = fileName.toStdString();
        if (!filePath.empty()) {
            fileSelected = true;
        }
    } else {
        fileSelected = true;
    }

    if (fileSelected) {
        levelEditor.store(filePath);
    }
}

void MainWindow::undo() {
	std::cout << "UNDO" << std::endl;
	levelEditor.undo();
    ui->graphicsView->update();
}

void MainWindow::redo() {
	std::cout << "REDO" << std::endl;
	levelEditor.redo();
    ui->graphicsView->update();
}

void MainWindow::sceneClicked(QMouseEvent* event, QPointF position) {
	float x;
	float y;
	if (selectedGameObject) {
		x = std::floor(position.x() -
				static_cast<float>(selectedGameObject->width)/2.0f + 0.5f);
		y = std::floor(-position.y() -
				static_cast<float>(selectedGameObject->height)/2.0f + 0.5f);
	} else {
		x = std::floor(position.x());
		y = std::floor(-position.y());
	}

	if (event->button() == Qt::RightButton) {
		levelEditor.removeGameObjectAtPosition(x, y);
	} else if (selectedGameObject && event->button() == Qt::LeftButton) {
		levelEditor.addGameObject(*selectedGameObject, x, y);
	} else {
		return;
	}

    ui->graphicsView->update();
}

void MainWindow::setupEditorGraphics() {
	levelEditor.setGraphicsView(*ui->graphicsView);

    QTransform M;
    M.scale(scale, scale);
    ui->graphicsView->setTransform(M);
    ui->graphicsView->update();
}

void MainWindow::updateTitle() {
    setWindowTitle(QString::fromStdString(levelEditor.getName() + " - LevelEditor"));
}

MainWindow::~MainWindow() {
    delete ui;
}
