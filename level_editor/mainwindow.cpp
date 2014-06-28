#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "newleveldialog.h"
#include "config.h"

#include <QFileDialog>
#include <QGraphicsItem>
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

    ui->graphicsView->setScene(&scene);

    updateTitle();
    updateScene();
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

// TODO: When we make introduce shortcuts for picking the game object we might need to change this...
void MainWindow::gameObjectPicked() {
    auto goAction = dynamic_cast<QAction*>(sender());
    selectedGameObject = const_cast<GameObject const*>(static_cast<GameObject*>(goAction->data().value<void*>())); // Follows from horrible shit!

    // Mark as selected.
    static_cast<QMenu*>(goAction->parentWidget())->menuAction()->setIcon(
            QIcon(QString::fromStdString(Config::getInstance().getAssetsPath() + "/" + selectedGameObject->imagePath)));

    ui->graphicsView->setCursor(QCursor(QPixmap(QString::fromStdString(getImagePath(*selectedGameObject)))));
}

void MainWindow::exit() {
    qApp->exit();
}

void MainWindow::newLevel() {
    NewLevelDialog nld;
    if (nld.exec()) {
        newLevel(nld.getEditor());
    }
}

void MainWindow::newLevel(LevelEditor const& editor) {
    levelEditor = editor;
    updateTitle();
    updateScene();
}

void MainWindow::openLevel() {
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Level"), ".", tr("Text files (*.json)"));
    filePath = fileName.toStdString();
    if (!filePath.empty()) {
        levelEditor = LevelEditor(filePath);
        updateTitle();
        updateScene();
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

void MainWindow::sceneClicked(QPointF position) {
	if (selectedGameObject) {
		auto x = std::floor(position.x() -
				static_cast<float>(selectedGameObject->width)/2.0f + 0.5f);
		auto y = std::floor(-position.y() -
				static_cast<float>(selectedGameObject->height)/2.0f + 0.5f);
		levelEditor.addTile(selectedGameObject->name, x, y);

		addGameObject(*selectedGameObject, x, y);
    	ui->graphicsView->update();
	}
}

void MainWindow::addGameObject(GameObject const& go, int x, int y) {
	auto graphicsItem =
		scene.addPixmap(QPixmap(QString::fromStdString(getImagePath(go))));
	graphicsItem->setPos(x, -y - static_cast<float>(go.height));
	graphicsItem->setScale(1.0f/scale);
}

void MainWindow::updateScene() {
    scene.clear();
    scene.setSceneRect(0, -levelEditor.getHeight()/2.0f, levelEditor.getWidth(), levelEditor.getHeight());

    QPen pen(QColor(0, 0, 255));
    pen.setStyle(Qt::DashLine);

    // Let's draw grid...
    for (double x = 0.0; x < levelEditor.getWidth(); x += 1.0) {
        scene.addLine(x, -levelEditor.getHeight()/2.0f, x, levelEditor.getHeight()/2.0f, pen);
    }
    for (double y = -static_cast<double>(levelEditor.getHeight())/2.0; y < levelEditor.getHeight()/2.0; y += 1.0) {
		if (y == 0) {
        	scene.addLine(0, y, levelEditor.getWidth(), y, QPen(QColor(255, 0 ,0)));
		} else {
        	scene.addLine(0, y, levelEditor.getWidth(), y, pen);
		}
    }

	for (auto& tile : levelEditor.getTiles()) {
		auto go = lookupGameObject(tile.name);
		if (go) {
			addGameObject(*go, tile.position.first, tile.position.second);
		}
	}

    QTransform M;
    M.scale(scale, scale);
    ui->graphicsView->setTransform(M);
    ui->graphicsView->update();
}

void MainWindow::updateTitle() {
    setWindowTitle(QString::fromStdString(levelEditor.getName() + " - LevelEditor"));
}

MainWindow::~MainWindow()
{
    delete ui;
}
