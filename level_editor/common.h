#ifndef COMMON_H
#define COMMON_H

#include <cstddef>
#include <string>

struct Category {
    std::string name;
    std::string iconPath;
};

struct GameObject {
    std::string name;
    std::string category;
    std::string imagePath;
	int width;
	int height;
};

#endif // COMMON_H
