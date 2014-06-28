#include "leveleditor.h"
#include <jsoncpp/json/json.h>
#include <jsoncpp/json/writer.h>

#include <iostream>
#include <fstream>

void LevelEditor::store(std::string const& path) {
    Json::Value root;
    root["name"] = name;
    root["width"] = static_cast<int>(width);
    root["height"] = static_cast<int>(height);

    Json::Value tileArr(Json::arrayValue);
    for (auto& tile: tiles) {
        Json::Value tileValue;
        tileValue["name"] = tile.name;
        tileValue["x"] = static_cast<int>(tile.position.first);
        tileValue["y"] = static_cast<int>(tile.position.second);

        tileArr.append(tileValue);
    }
    root["tiles"] = tileArr;

    std::ofstream file;
    file.open(path);
    Json::StyledWriter writer;
    file << writer.write(root);
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

    for (auto& tileValue : tilesArr) {
		auto name = tileValue["name"].asString();
        auto x = tileValue["x"].asInt();
        auto y = tileValue["y"].asInt();
		addTile(name, x, y);
	}
}
