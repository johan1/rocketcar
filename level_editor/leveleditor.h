#ifndef LEVELEDITOR_H
#define LEVELEDITOR_H

#include <algorithm>
#include <cstddef>
#include <string>
#include <vector>

class LevelEditor
{
public:
    struct Tile {
        std::string name;
        std::pair<int, int> position;
    };

    LevelEditor() : LevelEditor("Unnamed", 100, 50) {}

    LevelEditor(std::string const& name, int width, int height) :
        name(name), width(width), height(height) {}

    LevelEditor(std::string const& path);

    std::string const& getName() const { return name; }
    int getWidth() const { return width; }
    int getHeight() const { return height; }

    void addTile(std::string const& name, int x, int y) {
        addTile(name, std::make_pair(x, y));
    }

    void addTile(std::string const& name, std::pair<int, int> position) {
        tiles.push_back( Tile{name, position} );
    }

    void removeTile(int x, int y) {
        removeTile(std::make_pair(x, y));
    }

    void removeTile(std::pair<int, int> position) {
        std::remove_if(std::begin(tiles), std::end(tiles), [position](Tile &tile) {
            return tile.position == position;
        });
    }

    std::vector<Tile> const& getTiles() const {
        return tiles;
    }

    void store(std::string const& path);

private:
    std::string name;
    int width;
    int height;

    std::vector<Tile> tiles;
};

#endif // LEVELEDITOR_H
