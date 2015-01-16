#ifndef LEVELEDITOR_H
#define LEVELEDITOR_H

#include "common.h"
#include "editoraction.h"

#include <algorithm>
#include <cstddef>
#include <memory>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>

#include <QGraphicsView>
#include <QGraphicsScene>

class LevelEditor {
public:
    LevelEditor() : LevelEditor("Unnamed", 100, 50) {}

    LevelEditor(std::string const& name, int width, int height);

    LevelEditor(std::string const& path);

    std::string const& getName() const { return name; }
    int getWidth() const { return width; }
    int getHeight() const { return height; }

	void setGraphicsView(QGraphicsView &view) {
		view.setScene(scene.get());
	}

    void store(std::string const& path);

	void addGameObject(GameObject const& go, int x, int y);

	GameObject const* getGameObjectAtPosition(int x, int y) const {
		return getGameObjectAtPosition(std::make_pair(x, y));
	}

	GameObject const* getGameObjectAtPosition(std::pair<int, int> const& position) const {
		auto const it = std::find_if(begin(sceneObjects), end(sceneObjects),
				[this, &position](std::pair<EditorEntry, QGraphicsPixmapItem*> const& entry) {
			return entryOverlappingPosition(entry.first, position);
		});

		return it != sceneObjects.end() ? &(it->first.go) : nullptr;
	}

	void removeGameObjectAtPosition(int x, int y) {
		removeGameObjectAtPosition(std::make_pair(x, y));
	}

	void removeGameObjectAtPosition(std::pair<int, int> const& position);

	bool overlappingExistingGameObject(GameObject const& go, int x, int y) const {
		return overlappingExistingGameObject(go, std::make_pair(x, y));
	}

	bool overlappingExistingGameObject(GameObject const& go, std::pair<int, int> const& position) const {
		for (int i = 0; i < go.width; ++i) {
			for (int j = 0; j < go.height; ++j) {
				if (getGameObjectAtPosition(position.first + i, position.second + j)) {
					return true;
				}
			}
		}

		return false;
	}

	bool canUndo() const {
		return historyPosition > 0;
	}

	bool canRedo() const {
		return historyPosition < historyStack.size();
	}

	void undo();

	void redo();

private:
	struct EditorEntry {
		GameObject go;
		std::pair<int, int> position;

		EditorEntry() = default;

		EditorEntry(GameObject const& go, std::pair<int, int> position) :
				go(go), position(position) {}
	};

    std::string name;
    int width;
    int height;

	std::vector<std::pair<EditorEntry, QGraphicsPixmapItem*>> sceneObjects;

	std::unique_ptr<QGraphicsScene> scene;

	std::size_t historyPosition = 0;
	std::vector<std::pair<EditorEntry, bool>> historyStack;

	void setupScene();

	bool entryOverlappingPosition(EditorEntry const& entry, std::pair<int, int> const& position) const {
		return (position.first >= entry.position.first &&
				position.first < entry.position.first + static_cast<int>(entry.go.width) &&
				position.second >= entry.position.second &&
				position.second < entry.position.second + static_cast<int>(entry.go.height));
	}

	void addEditorEntry(EditorEntry const& editorEntry);
	void removeEditorEntry(EditorEntry const& editorEntry);
};

#endif // LEVELEDITOR_H
