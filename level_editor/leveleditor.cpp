#include "leveleditor.h"

#include "config.h"

#include <jsoncpp/json/json.h>
#include <jsoncpp/json/writer.h>

#include <iostream>
#include <fstream>
#include <QPen>
#include <QGraphicsPixmapItem>

static const float scale = 1.0f/64.0f;

LevelEditor::LevelEditor(std::string const& name, int width, int height) : 
		name(name), width(width), height(height), historyPosition(0) {
	setupScene();
}

LevelEditor::LevelEditor(std::string const& path) {
    Json::Value root;
    Json::Reader reader;

    std::ifstream file;
    file.open(path);
    reader.parse(file, root);

    name = root["name"].asString();
    width = root["width"].asInt();
    height = root["height"].asInt();
    Json::Value tilesArr = root["tiles"];

	setupScene();

    for (auto& tileValue : tilesArr) {
		auto name = tileValue["name"].asString();
        auto x = tileValue["x"].asInt();
        auto y = tileValue["y"].asInt();

		auto go = lookupGameObject(name);
		if (go) {
			addGameObject(*go, x, y);
		}
	}
}

void LevelEditor::setupScene() {
	scene = std::unique_ptr<QGraphicsScene>(new QGraphicsScene());
    scene->setSceneRect(0, -static_cast<float>(height)/2.0f, width, height);

    // Let's draw grid...
    QPen pen(QColor(0, 0, 255));
    pen.setStyle(Qt::DashLine);
    for (double x = 0.0; x < width; x += 1.0) {
        scene->addLine(x, -height/2.0f, x, height/2.0f, pen);
    }
    for (double y = -static_cast<double>(height)/2.0; y < static_cast<double>(height)/2.0; y += 1.0) {
		if (y == 0) {
        	scene->addLine(0, y, width, y, QPen(QColor(255, 0 ,0)));
		} else {
        	scene->addLine(0, y, width, y, pen);
		}
    }
}

void LevelEditor::addGameObject(GameObject const& go, int x, int y) {
	if (overlappingExistingGameObject(go, x, y)) {
		std::cout << "Unable to add game object position occupied" << std::endl;
		return;
	}

	EditorEntry entry(go, std::make_pair(x, y));
	addEditorEntry(entry);

	// Let's erase history after history position.
	historyStack.erase(historyStack.begin() + historyPosition, historyStack.end());
	historyStack.push_back(std::make_pair(entry, true));
	historyPosition = historyStack.size();
}

void LevelEditor::addEditorEntry(EditorEntry const& editorEntry) {
	auto graphicsItem =
			scene->addPixmap(QPixmap(QString::fromStdString(getImagePath(editorEntry.go))));

	graphicsItem->setPos(editorEntry.position.first,
			-editorEntry.position.second - static_cast<float>(editorEntry.go.height));
	graphicsItem->setScale(scale);

	sceneObjects.push_back(std::make_pair(editorEntry, graphicsItem));
}

void LevelEditor::removeGameObjectAtPosition(std::pair<int, int> const& position) {
	auto it = std::find_if(sceneObjects.begin(), sceneObjects.end(),
			[this, &position](std::pair<EditorEntry, QGraphicsPixmapItem*> const& sceneObject) {
		return entryOverlappingPosition(sceneObject.first, position);
	});

	if (it != sceneObjects.end()) {
		removeEditorEntry(it->first);
		historyStack.push_back(std::make_pair(it->first, false));
		historyPosition = historyStack.size();
	}
}

void LevelEditor::removeEditorEntry(EditorEntry const& editorEntry) {
	auto it = std::find_if(sceneObjects.begin(), sceneObjects.end(),
			[this, &editorEntry](std::pair<EditorEntry, QGraphicsPixmapItem*> const& sceneObject) {
		return entryOverlappingPosition(sceneObject.first, editorEntry.position);
	});

	if (it != sceneObjects.end()) {
		scene->removeItem(it->second);
		sceneObjects.erase(it);
	}
}

void LevelEditor::undo() {
	std::cout << "Undo1, history size: " << historyStack.size() << ", historyPosition: "
			<< historyPosition << std::endl;

	if (historyPosition <= 0) {
		return; // We're already back at the beginning
	}

	auto& historyItem = historyStack[--historyPosition];
	if (historyItem.second) { // Undo insertion
		std::cout << "Undo insertion" << std::endl;
		removeEditorEntry(historyItem.first);
	} else { // Undo remove
		std::cout << "Undo remove" << std::endl;
		addEditorEntry(historyItem.first);
	}

	std::cout << "Undo, history size: " << historyStack.size() << ", historyPosition: "
			<< historyPosition << std::endl;
}

void LevelEditor::redo() {
	std::cout << "Redo1, history size: " << historyStack.size() << ", historyPosition: "
			<< historyPosition << std::endl;
	if (historyPosition >= historyStack.size()) {
		return; // We're already end of history
	}

	auto& historyItem = historyStack[historyPosition++];
	if (historyItem.second) { // Redo insertion
		std::cout << "Redo insertion" << std::endl;
		addEditorEntry(historyItem.first);
	} else { // Redo remove
		std::cout << "Redo remove" << std::endl;
		removeEditorEntry(historyItem.first);
	}

	std::cout << "Redo2, history size: " << historyStack.size() << ", historyPosition: "
			<< historyPosition << std::endl;
}

void LevelEditor::store(std::string const& path) {
    Json::Value root;
    root["name"] = name;
    root["width"] = static_cast<int>(width);
    root["height"] = static_cast<int>(height);

    Json::Value tileArr(Json::arrayValue);
    for (auto& sceneObject : sceneObjects) {
        Json::Value tileValue;
        tileValue["name"] = sceneObject.first.go.name;

        tileValue["x"] = static_cast<int>(sceneObject.first.position.first);
        tileValue["y"] = static_cast<int>(sceneObject.first.position.second);
        tileArr.append(tileValue);
    }
    root["tiles"] = tileArr;

    std::ofstream file;
    file.open(path);
    Json::StyledWriter writer;
    file << writer.write(root);
}
