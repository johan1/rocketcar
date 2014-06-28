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
    std::size_t width;
    std::size_t height;
};

#endif // COMMON_H
