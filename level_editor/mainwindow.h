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
	float scale;

    std::vector<std::unique_ptr<QMenu>> gameObjectGroups;

    GameObject const* selectedGameObject;

    void setupToolbar();
    void updateTitle();
    void setupEditorGraphics();
    void newLevel(LevelEditor && levelEditor);
    void loadGameItems();

private slots:
    void newLevel();
    void openLevel();
    void saveLevel();
    void exit();
	void undo();
	void redo();

    void gameObjectPicked();
	void sceneClicked(QMouseEvent* event, QPointF position);
};

#endif // MAINWINDOW_H
