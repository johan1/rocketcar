#include "config.h"

#include <QDir>

#include <iostream>
#include <fstream>
#include <string>

#include <jsoncpp/json/json.h>

Config::Config() {
    Json::Value root;
    Json::Reader reader;
    std::ifstream file;

    auto configFile = QDir::homePath().append("/.rocketcar_editor").toStdString();
    file.open(configFile);
    reader.parse(file, root);
    assets_path = root["assets_path"].asString();

	loadGameItems();
}

void Config::loadGameItems() {
    auto configFilePath = assets_path + "/game_items.json";
    std::ifstream configFile;
    Json::Reader reader;
    Json::Value root;
    configFile.open(configFilePath);
    reader.parse(configFile, root);

    auto categoryArr = root["categories"];
    for (auto& categoryValue : categoryArr) {
		auto name = categoryValue["name"].asString();
		auto iconPath = categoryValue["icon"].asString();
		categories.push_back(Category {name, iconPath});
    }

    auto gameObjectArr = root["gameobjects"];
    for (auto& gameObjectValue : gameObjectArr) {
        auto name = gameObjectValue["name"].asString();
        auto category = gameObjectValue["category"].asString();
        auto image = gameObjectValue["image"].asString();
        int width = gameObjectValue["width"].asInt();
        int height = gameObjectValue["height"].asInt();
		gameObjects.push_back(GameObject {name, category, image, width, height});
    }
}

Config const& Config::getInstance() {
    static Config instance;
    return instance;
}
