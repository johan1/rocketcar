#ifndef CONFIG_H
#define CONFIG_H

#include <algorithm>
#include <string>
#include <vector>
#include <cstddef>

#include "common.h"
#include "config.h"

// Well this might not be the best name. It kinda holds
class Config {
public:
    static Config const& getInstance();

	std::vector<Category> const& getCategories() const { return categories; }
	std::vector<GameObject> const& getGameObjects() const { return gameObjects; }
	std::string const& getAssetsPath() const { return assets_path; }

	GameObject const* lookupGameObject(std::string const& name) const {
		auto it = std::find_if(begin(gameObjects), end(gameObjects), [&name](GameObject const& go) {
			return go.name == name;
		});
		return it != gameObjects.end() ? &*it : nullptr;
	}

private:
    std::string assets_path; // TODO: We should rename this.
    std::vector<GameObject> gameObjects;
    std::vector<Category> categories;

    Config();
    void loadGameItems();
};

// TODO: Should we introduce shortcuts for accessing config. E.g. free functions for getCategories, etc..?

inline
GameObject const* lookupGameObject(std::string const& name) {
    return Config::getInstance().lookupGameObject(name); 
}

inline
std::string getPathToAsset(std::string const& relativePath) {
    return Config::getInstance().getAssetsPath() + "/" + relativePath;
}

inline
std::string getIconPath(Category const& category) {
    return getPathToAsset(category.iconPath);
}

inline
std::string getImagePath(GameObject const& go) {
    return getPathToAsset(go.imagePath);
}

#endif // CONFIG_H
