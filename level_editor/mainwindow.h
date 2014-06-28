#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "leveleditor.h"
#include "common.h"

#include <QGraphicsScene>
#include <QMainWindow>
#include <QMenu>

#include <memory>
#include <vector>

namespace Ui {
class MainWindow;
}

class LevelEditor;

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    LevelEditor levelEditor;
    std::string filePath;
    QGraphicsScene scene;
	float scale;

    std::vector<std::unique_ptr<QMenu>> gameObjectGroups;

    GameObject const* selectedGameObject;

    void setupToolbar();
    void updateTitle();
    void updateScene();
    void newLevel(LevelEditor const& levelEditor);
    void loadGameItems();

	void addGameObject(GameObject const& go, int x, int y);

private slots:
    void newLevel();
    void openLevel();
    void saveLevel();
    void exit();
    void gameObjectPicked();
	void sceneClicked(QPointF position);
};

#endif // MAINWINDOW_H
